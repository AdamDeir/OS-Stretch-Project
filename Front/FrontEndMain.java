import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.Socket;
import java.util.concurrent.atomic.AtomicBoolean;

public class FrontEndMain extends JFrame {
    private Socket theSocket;
    private PrintWriter socketOutput;
    private BufferedReader socketInput;
    private JTextArea messageArea; // For displaying messages and server responses
    private AtomicBoolean terminate = new AtomicBoolean(false); // Flag to control the listening thread

    public FrontEndMain() {
        // Initialize GUI Components
        initializeGUI();

        // Setup connection and start listening
        connectToServer("127.0.0.1", 3000);
    }

    private void initializeGUI() {
        setTitle("Network Game Console");
        setSize(600, 400);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        messageArea = new JTextArea();
        messageArea.setEditable(false);
        JScrollPane scrollPane = new JScrollPane(messageArea);
        JTextField inputField = new JTextField(40);
        JButton sendButton = new JButton("Send");

        // Action to send a message
        sendButton.addActionListener(e -> sendMessage(inputField.getText()));
        inputField.addActionListener(e -> sendMessage(inputField.getText())); // Send message on Enter
		inputField.setText("");

        JPanel bottomPanel = new JPanel();
        bottomPanel.add(inputField);
        bottomPanel.add(sendButton);

        getContentPane().add(scrollPane, BorderLayout.CENTER);
        getContentPane().add(bottomPanel, BorderLayout.SOUTH);

        setVisible(true);
    }

    private void connectToServer(String host, int port) {
        try {
            if (theSocket != null && !theSocket.isClosed()) {
                theSocket.close(); // Ensure old connections are closed
            }
            theSocket = new Socket(host, port);
            socketOutput = new PrintWriter(theSocket.getOutputStream(), true);
            socketInput = new BufferedReader(new InputStreamReader(theSocket.getInputStream()));
            messageArea.append("Connected to the server.\n");

            // Start listening for messages from the server
            startListeningThread();
        } catch (IOException e) {
            messageArea.append("Failed to connect: " + e.getMessage() + "\n");
            terminate.set(true);
        }
    }

    private void startListeningThread() {
		System.out.println("print 1");
        Thread listenThread = new Thread(() -> {
            try {
				System.out.println("Print 2");
                String line;
				System.out.println(" Terminate.get: ");
				System.out.println(terminate.get());
                while (!terminate.get() && (line = socketInput.readLine()) != null) {
                    messageArea.append("Server: " + line + "\n");
					System.out.println("print 3");
                }
            } catch (IOException e) {
                if (!terminate.get()) {
                    messageArea.append("Lost connection to server: " + e.getMessage() + "\n");
                }
            }
        });
        listenThread.start();
		System.out.println("we out");
    }

    private void sendMessage(String message) {
        if (message.isEmpty()) {
            return; // Don't send empty messages
        }
        socketOutput.println(message);
        messageArea.append("You: " + message + "\n");
        if (message.equalsIgnoreCase("done")) {
            terminate.set(true);
            try {
                theSocket.close();
                messageArea.append("Disconnected.\n");
            } catch (IOException e) {
                messageArea.append("Error closing connection: " + e.getMessage() + "\n");
            }
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(FrontEndMain::new);
    }
}
