package com.milan338.cryptowearcompanion;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;

public class ErrorDialogActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();
        new AlertDialog.Builder(this)
            .setTitle(intent.getStringExtra("title"))
            .setMessage(intent.getStringExtra("message"))
            .setCancelable(false)
            .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    finishAffinity();
                    System.exit(1);
                }
            })
            .show();
    }
}
