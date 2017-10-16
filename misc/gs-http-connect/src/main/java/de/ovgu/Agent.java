package de.ovgu;

import com.google.gson.Gson;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

import static java.lang.System.exit;

public class Agent implements Runnable {

    class GatewayInfo {
        public int use_port = 0;
    }

    Thread thread;
    int gatewayPort;
    GatewayInfo gatewayInfo;

    public Agent(int gatewayPort) {
        thread = new Thread(this);
        this.gatewayPort = gatewayPort;

        boolean success = false;
        while (!success) {
            try {
                URL url = new URL("http://localhost:" + gatewayPort + "/api/1.0/");
                HttpURLConnection connection = (HttpURLConnection) url.openConnection();
                connection.setRequestMethod("GET");
                connection.setRequestProperty("User-Agent", "de-ovgu-http-connect/1.0");

                int rc = connection.getResponseCode();

                if (rc == 200) {
                    BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
                    StringBuilder sb = new StringBuilder();
                    String line;
                    while ((line = reader.readLine()) != null) {
                        sb.append(line);
                    }
                    reader.close();

                    Gson gson = new Gson();
                    gatewayInfo = gson.fromJson(sb.toString(), GatewayInfo.class);
                    success = true;
                } else {
                    System.err.println("HTTP GET rejected: " + rc);
                    exit(0);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public void start() {
        thread.start();
    }

    public void sync() throws InterruptedException {
        thread.join();
    }


    public void run() {
        boolean success = false;
        while (!success) {
            try {
                URL url = new URL("http://localhost:" + gatewayInfo.use_port + "/api/1.0/nodes");
                HttpURLConnection connection = (HttpURLConnection) url.openConnection();
                connection.setRequestMethod("POST");
                connection.setRequestProperty("User-Agent", "de-ovgu-http-connect/1.0");

                int rc = connection.getResponseCode();

                if (rc == 200) {
                    BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
                    StringBuilder sb = new StringBuilder();
                    String line;
                    while ((line = reader.readLine()) != null) {
                        sb.append(line);
                    }
                    reader.close();
                    // System.out.println(sb);

                    //  Gson gson = new Gson();
                    //  gatewayInfo = gson.fromJson(sb.toString(), GatewayInfo.class);
                    //  System.err.println(gatewayInfo.use_port);
                    success = true;
                } else {
                    System.err.println("HTTP GET rejected: " + rc);
                    exit(0);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

}
