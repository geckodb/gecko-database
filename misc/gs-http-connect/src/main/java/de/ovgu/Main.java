package de.ovgu;

import com.oracle.javafx.jmx.json.JSONReader;

import javax.net.ssl.HttpsURLConnection;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

public class Main {

    static final int NUM_SERVER_SOCKETS = 200;
    static final int MAX_SAMPLE = 5;

    public static void main(String[] args) {

        System.out.println("timestamp;num_server_sockets;num_agents;gateway_time_ms;gateway_latency_ms;gateway_throughput_ms;request_time_ms;request_latency_ms;request_throughput_ms;sample_id");

        float gatewayTimeMs = 0;
        float requestTimeMs = 0;

        for (int num_agents = 2000; num_agents < 2001; num_agents += 25) {
            for (int sample_id = 0; sample_id < MAX_SAMPLE; sample_id++) {
                System.out.println(sample_id);
                Agent agents[] = new Agent[num_agents];

                long gatewayStart = System.currentTimeMillis();
                for (int i = 0; i < num_agents; i++) {
                    agents[i] = new Agent(35497);
                }
                long gatewayEnd = System.currentTimeMillis();

                long requestStart = System.currentTimeMillis();
                for (int i = 0; i < num_agents; i++) {
                    agents[i].start();
                }

                for (int i = 0; i < num_agents; i++) {
                    try {
                        agents[i].sync();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                long requestEnd = System.currentTimeMillis();

                gatewayTimeMs += (gatewayEnd - gatewayStart);
                requestTimeMs += (requestEnd - requestStart);
            }

            gatewayTimeMs /= (float) MAX_SAMPLE;
            requestTimeMs /= (float) MAX_SAMPLE;
            float gatewayLatencyMs = gatewayTimeMs / (float) num_agents;
            float gatewayThroughputMs = num_agents / (gatewayTimeMs / 1000.0f);

            float requestLatencyMs = requestTimeMs / (float) num_agents;
            float requestThroughputMs = num_agents / (requestTimeMs / 1000.0f);

                System.out.println(
                        System.currentTimeMillis() + ";" +
                                NUM_SERVER_SOCKETS + ";" +
                                num_agents + ";" +
                                gatewayTimeMs + ";" +
                                gatewayLatencyMs + ";" +
                                gatewayThroughputMs + ";" +
                                requestTimeMs + ";" +
                                requestLatencyMs + ";" +
                                requestThroughputMs);
        }
    }

}
