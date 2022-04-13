package org.salieff;

import android.net.Uri;
import android.os.Parcelable;
import android.provider.DocumentsContract;
import android.content.Intent;
import android.app.Activity;
import android.content.Context;
import android.content.UriPermission;
import android.os.ParcelFileDescriptor;
import android.database.Cursor;
import android.util.Log;

public class SafAdapter
{
    public static final int RequestCode = 513375;
    public static Uri RootUri = null;

    public static boolean rootUriPermissionGranted(Context context)
    {
        for (UriPermission p : context.getContentResolver().getPersistedUriPermissions())
        {
            if (p.getUri().toString().equals("content://com.android.externalstorage.documents/tree/primary%3A") && p.isReadPermission() && p.isWritePermission())
            {
                RootUri = p.getUri();
                return true;
            }
        }

        return false;
    }

    public static Intent intentForRootUriPermissionRequest()
    {
        Intent intent = new Intent("android.intent.action.OPEN_DOCUMENT_TREE");

        Uri intentRootUri = DocumentsContract.buildDocumentUri("com.android.externalstorage.documents", "primary:");
        intent.putExtra("android.provider.extra.INITIAL_URI", (Parcelable)intentRootUri);

        intent.putExtra("android.content.extra.SHOW_ADVANCED", true);
        intent.putExtra("android.content.extra.FANCY", true);
        intent.putExtra("android.content.extra.SHOW_FILESIZE", true);

        return intent;
    }

    public static void takeRootUriPermission(Context context, Intent intent)
    {
        Uri intentRootUri = intent.getData();
        if (!intentRootUri.toString().equals("content://com.android.externalstorage.documents/tree/primary%3A"))
            return;

        context.getContentResolver().takePersistableUriPermission(intentRootUri, Intent.FLAG_GRANT_WRITE_URI_PERMISSION | Intent.FLAG_GRANT_READ_URI_PERMISSION);
        RootUri = intentRootUri;
    }

    public static boolean createFolder(Context context, String rootFolderName, String subFolderName)
    {
        try
        {
            if (folderSize(context, rootFolderName + "/" + subFolderName) >= 0)
                return true;

            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + rootFolderName);
            Uri subFolderUri = DocumentsContract.createDocument(context.getContentResolver(), rootFolderUri, DocumentsContract.Document.MIME_TYPE_DIR, subFolderName);

            return true;
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return false;
    }

    public static int openFile(Context context, String rootFolderName, String fileName, String mode)
    {
        try
        {
            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + rootFolderName);
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + rootFolderName + "/" + fileName);
            ParcelFileDescriptor fileDiscriptor = context.getContentResolver().openFileDescriptor(fileUri, mode);
            return fileDiscriptor.detachFd();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return -1;
    }

    public static int createFile(Context context, String rootFolderName, String fileName, String mode)
    {
        try
        {
            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + rootFolderName);
            int fd = openFile(context, rootFolderName, fileName, mode);
            if (fd >= 0)
                return fd;

            Uri fileUri = DocumentsContract.createDocument(context.getContentResolver(), rootFolderUri, "application/octet-stream", fileName);
            ParcelFileDescriptor fileDiscriptor = context.getContentResolver().openFileDescriptor(fileUri, mode);
            return fileDiscriptor.detachFd();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return -1;
    }

    public static boolean deleteFile(Context context, String fileName)
    {
        try
        {
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + fileName);
            return DocumentsContract.deleteDocument(context.getContentResolver(), fileUri);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return false;
    }

    public static int folderSize(Context context, String folderName)
    {
        int retValue = -1;
        Cursor cursor = null;

        try
        {
            Uri folderUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + folderName);
            cursor = context.getContentResolver().query(folderUri, new String[]{DocumentsContract.Document.COLUMN_DOCUMENT_ID}, null, null, null);

            if (cursor == null)
                retValue = -1;
            else
                retValue = cursor.getCount();
        }
        catch (Exception e)
        {
            e.printStackTrace();
            retValue = -1;
        }
        finally
        {
            if (cursor != null)
                cursor.close();
        }

        return retValue;
    }

    public static long fileSize(Context context, String fileName)
    {
        try
        {
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + fileName);
            ParcelFileDescriptor fileDiscriptor = context.getContentResolver().openFileDescriptor(fileUri, "r");
            return fileDiscriptor.getStatSize();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return -1;
    }
}
