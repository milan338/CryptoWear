package com.milan338.cryptowearcompanion;

import java.util.HashMap;
import java.util.Map;

import androidx.appcompat.app.AppCompatActivity;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputType;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.EditText;
import android.text.TextWatcher;

import org.json.JSONException;
import org.json.JSONObject;

public class MainActivity extends AppCompatActivity {
    public static final HashMap<String, String> keys = new HashMap<>();
    protected SharedPreferences keyStore;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        keyStore = getPreferences(MODE_PRIVATE);
        // Get main vertical layout
        LinearLayout layout = findViewById(R.id.layout_main);
        // Elements to add - UI string, key ID, secret ID
        String[][] key_data = {
                {"Coinbase", "coinbase_key", "coinbase_secret"},
                {"CoinSpot", "coinspot_key", "coinspot_secret"}
        };
        // Add all providers to scroll view
        for (String[] provider : key_data) {
            // Add exchange
            TextView tv = new TextView(this);
            tv.setText(provider[0]);
            layout.addView(tv);
            // Add key inputs
            addInput("API key", provider[1], layout);
            addInput("API secret", provider[2], layout);
        }
        // Add helper text
        TextView tv = new TextView(this);
        tv.setText("Go to settings -> sync keys on your wearable once finished");
        layout.addView(tv);
        // Add button listener
        Button updateBtn = findViewById(R.id.save_btn);
        updateBtn.setOnClickListener(view -> {
            SharedPreferences.Editor editor = keyStore.edit();
            for (Map.Entry<String, String> entry : keys.entrySet())
                editor.putString(entry.getKey(), entry.getValue());
            editor.apply();
        });
    }

    protected void addInput(String hint, String key, LinearLayout layout) {
        EditText input = new EditText(this);
        // Set input type to password to hide the entered keys
        input.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
        // Update map with new data whenever keys are entered
        input.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) { }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) { }

            @Override
            public void afterTextChanged(Editable s) {
                String userInput = input.getText().toString();
                keys.put(key, userInput);
            }
        });
        // Set text hint
        input.setHint(hint);
        // Add input to scroll view
        layout.addView(input);
        // Add existing stored key data
        String keyValue = keyStore.getString(key, null);
        if (keyValue != null)
            input.setText(keyValue);
        else
            input.setText("");
    }

    public static JSONObject getKeys() {
        JSONObject obj = new JSONObject();
        try {
            // Iterate through map and update all fields
            for (Map.Entry<String, String> entry : keys.entrySet())
                obj.put(entry.getKey(), entry.getValue());
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return obj;
    }
}
