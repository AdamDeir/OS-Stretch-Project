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
	private String username;



	
    public FrontEndMain() {
        // Initialize GUI Components
        initializeGUI();

        // Setup connection and start listening
        connectToServer("127.0.0.1", 3000);

		this.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				terminate.set(true); // Signal threads to terminate
				try {
					if (theSocket != null && !theSocket.isClosed()) {
						theSocket.close(); // Close the socket connection
					}
					messageArea.append("Disconnected.\n");
				} catch (IOException ex) {
					messageArea.append("Error closing connection: " + ex.getMessage() + "\n");
				} finally {
					// Ensure the application exits cleanly
					System.exit(0);
				}
			}
		});
    }






	//CLIENT STUFF

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

















	//SERVER STUFF
    private void connectToServer(String host, int port) {
        try {
            if (theSocket != null && !theSocket.isClosed()) {
                theSocket.close(); // Ensure old connections are closed
            }
            theSocket = new Socket(host, port);
            socketOutput = new PrintWriter(theSocket.getOutputStream(), true);
            socketInput = new BufferedReader(new InputStreamReader(theSocket.getInputStream()));
            messageArea.append("Connected to the server.\n");
			username = JOptionPane.showInputDialog("Enter your username:");
            startListeningThread();
        } catch (IOException e) {
            messageArea.append("Failed to connect: " + e.getMessage() + "\n");
            terminate.set(true);
        }
    }

    private void startListeningThread() {
        Thread listenThread = new Thread(() -> {
            try {
                String line;
				boolean startMsg = false;
				boolean spectating = false;
				
                while (!terminate.get() && (line = socketInput.readLine()) != null) {
					System.out.println(line);
					if(line.equals("Server asks: Ready to start?")){
						startMsg = true;
					};
					String questionResponse = JOptionPane.showInputDialog(line);
					if(startMsg){
						questionResponse = "Good Luck!";
						startMsg = false;
					}
					if (!spectating) {
						messageArea.append(username + questionResponse + "\n");

						String questionForOpp = JOptionPane.showInputDialog("Question for Opponent:");
						sendMessage(username + " says: " + questionResponse + " and asks: " + questionForOpp);
					}

					
                }
            } catch (IOException e) {
                if (!terminate.get()) {
                    messageArea.append("Lost connection to server: " + e.getMessage() + "\n");
                }
            }
        });
        listenThread.start();
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
