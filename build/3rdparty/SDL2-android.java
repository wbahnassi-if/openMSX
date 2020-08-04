package org.openmsx.android.openmsx;

import org.libsdl.app.SDLActivity; 
import android.content.res.*;
import android.os.*;
import android.util.*;
import java.io.*;
import java.util.zip.*;

public class openMSXActivity extends SDLActivity {
	private static final String TAG = "openMSXActivity";

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "hidapi",
            "SDL2",
            // "SDL2_image",
            // "SDL2_mixer",
            // "SDL2_net",
            // "SDL2_ttf",
            "openmsx" // Keep this last so getMainSharedObject uses it as the main library
        };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
		File dataLocation = new File(this.getFilesDir(),"openmsx_system");
		if (!dataLocation.exists())
			dataLocation.mkdirs();
		if (dataLocation.list().length == 0) {
			try {
				Log.v(TAG, "begin unpack app data to: " + dataLocation.getAbsolutePath());
				AssetManager manager = this.getAssets();
				InputStream is = manager.open("appdata.zip");
				unpackZip(is, dataLocation);
				is.close();
				Log.v(TAG, "done unpack app data");
			}
			catch (Exception ex) {
				Log.v(TAG, "Failed to unpack app data. " + ex.toString());
			}
		}
		else Log.v(TAG, "app data hopefully up-to-date");
	}
	
	private void unpackZip(InputStream is, File targetPath) throws Exception {       
		ZipInputStream zis = new ZipInputStream(new BufferedInputStream(is));
		byte[] buffer = new byte[1024];
		ZipEntry ze;
		int count;
		while ((ze = zis.getNextEntry()) != null)
		{
			String filename = ze.getName();
			if (ze.isDirectory()) {
				File fmd = new File(targetPath, filename);
				fmd.mkdirs();
				continue;
			}

			FileOutputStream fout = new FileOutputStream(new File(targetPath, filename));
			while ((count = zis.read(buffer)) != -1)
				fout.write(buffer, 0, count);
			fout.close();
			zis.closeEntry();
		}
		zis.close();
	}
}
