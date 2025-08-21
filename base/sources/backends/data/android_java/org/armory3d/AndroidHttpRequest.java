package arm;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

class AndroidHttpRequest {

	public static byte[] androidHttpRequest(String address) throws Exception {
		// https://developer.android.com/reference/java/net/HttpURLConnection.html
		URL url = new URL(address);
		HttpURLConnection urlConnection = (HttpURLConnection)url.openConnection();
		InputStream in = new BufferedInputStream(urlConnection.getInputStream());

		ByteArrayOutputStream buffer = new ByteArrayOutputStream();
		int i;
		byte[] data = new byte[4];
		while ((i = in.read(data, 0, data.length)) != -1) {
			buffer.write(data, 0, i);
		}
		buffer.flush();
		byte[] result = buffer.toByteArray();

		urlConnection.disconnect();
		return result;
	}
}
