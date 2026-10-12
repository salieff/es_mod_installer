package org.salieff;

import android.app.*;
import android.content.*;
import android.database.Cursor;
import android.os.Build;

public class DownloadCompleteReceiver extends BroadcastReceiver {
    private static final String CHANNEL_ID = "downloads_done_v2";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();

        if (DownloadManager.ACTION_NOTIFICATION_CLICKED.equals(action)) {
            Intent launch = context.getPackageManager().getLaunchIntentForPackage(context.getPackageName());
            if (launch != null) {
                launch.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(launch);
            }
            return;
        }

        if (!DownloadManager.ACTION_DOWNLOAD_COMPLETE.equals(action))
            return;

        if (AppState.isActive())
            return;   // приложение на переднем плане: завершение и так видно в GUI

        long id = intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, -1);
        if (id < 0)
            return;

        DownloadManager dm = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        String title;
        boolean ok;
        try (Cursor c = dm.query(new DownloadManager.Query().setFilterById(id))) {
            if (c == null || !c.moveToFirst())
                return;
            title = c.getString(c.getColumnIndexOrThrow(DownloadManager.COLUMN_TITLE));
            ok = c.getInt(c.getColumnIndexOrThrow(DownloadManager.COLUMN_STATUS)) == DownloadManager.STATUS_SUCCESSFUL;
        }

        NotificationManager nm = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

        Notification.Builder b;
        if (Build.VERSION.SDK_INT >= 26) {
            nm.createNotificationChannel(new NotificationChannel(CHANNEL_ID, "Downloads", NotificationManager.IMPORTANCE_HIGH));
            b = new Notification.Builder(context, CHANNEL_ID);
        } else {
            b = new Notification.Builder(context)
                    .setPriority(Notification.PRIORITY_HIGH)
                    .setDefaults(Notification.DEFAULT_ALL);   // на API 21–25 всплытие требует звука или вибрации
        }

        Intent launch = context.getPackageManager().getLaunchIntentForPackage(context.getPackageName());
        PendingIntent pi = PendingIntent.getActivity(context, (int) id, launch, PendingIntent.FLAG_IMMUTABLE | PendingIntent.FLAG_UPDATE_CURRENT);

        int iconRes = context.getResources().getIdentifier("ic_stat_download", "drawable", context.getPackageName());
        if (iconRes == 0)
            iconRes = context.getApplicationInfo().icon;   // запасной вариант

        b.setSmallIcon(iconRes)
         .setContentTitle(title)
         .setContentText(ok ? "Download complete, tap to unpack" : "Download failed")
         .setContentIntent(pi)
         .setAutoCancel(true);

        nm.notify((int) id, b.build());
    }
}
