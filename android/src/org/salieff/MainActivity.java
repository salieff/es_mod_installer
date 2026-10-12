package org.salieff;

import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.WindowInsets;
import android.view.KeyEvent;

import org.qtproject.qt5.android.bindings.QtActivity;

public class MainActivity extends QtActivity {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final View content = findViewById(android.R.id.content);
        content.setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
            @Override
            public WindowInsets onApplyWindowInsets(View v, WindowInsets insets) {
                int l, t, r, b;
                if (Build.VERSION.SDK_INT >= 30) {
                    android.graphics.Insets i = insets.getInsets(
                            WindowInsets.Type.systemBars()
                                    | WindowInsets.Type.displayCutout()
                                    | WindowInsets.Type.ime());
                    l = i.left;
                    t = i.top;
                    r = i.right;
                    b = i.bottom;
                    v.setPadding(l, t, r, b);
                    return WindowInsets.CONSUMED;
                } else {
                    v.setPadding(insets.getSystemWindowInsetLeft(),
                            insets.getSystemWindowInsetTop(),
                            insets.getSystemWindowInsetRight(),
                            insets.getSystemWindowInsetBottom());
                    return insets.consumeSystemWindowInsets();
                }
            }
        });
        content.requestApplyInsets();

        if (Build.VERSION.SDK_INT >= 33) {
            getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
                    android.window.OnBackInvokedDispatcher.PRIORITY_DEFAULT,
                    new android.window.OnBackInvokedCallback() {
                        @Override
                        public void onBackInvoked() {
                            MainActivity.this.onKeyDown(KeyEvent.KEYCODE_BACK,
                                    new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_BACK));
                            MainActivity.this.onKeyUp(KeyEvent.KEYCODE_BACK,
                                    new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_BACK));
                        }
                    });
        }
    }
}
