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
    //private JTextArea messageArea; // For displaying messages and server responses
    private AtomicBoolean terminate = new AtomicBoolean(false); // Flag to control the listening thread
	private String username;



	
    public FrontEndMain() {
        // Initialize GUI Components
        initializeGUI();

        // Setup connection and start listening
        connectToServer("127.0.0.1", 3000);


    }


	private void initializeGUI() {
		setTitle("Network Game Console");
		setSize(800, 600); // Adjust the size to fit the game board
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	
		setLayout(new BorderLayout());
	
		initializeCharacterGrid(); // Initialize the game board

	
		// Initialize and add other components like message area, input field, send button, etc., here

		setVisible(true);
	}
	
	private void initializeCharacterGrid() {
		JPanel characterGrid = new JPanel(new GridLayout(3, 4, 10, 10)); // Creates a grid with 3 rows and 4 columns
		characterGrid.setBorder(BorderFactory.createTitledBorder("Guess Who Characters"));
	
		// Instead of loading images, just use numbered buttons for testing
		for (int i = 1; i <= 12; i++) {
			JButton button = new JButton(String.valueOf(i)); // Use the index as the button label
			final int index = i; // Necessary for use within lambda
			button.addActionListener(e -> characterButtonClicked(index));
			characterGrid.add(button);
		}
	
		getContentPane().add(characterGrid, BorderLayout.EAST); // Add the character grid to the main window
	}
	
	// This method is triggered when any character button is clicked
	private void characterButtonClicked(int characterIndex) {
		JOptionPane.showMessageDialog(this, "Character " + characterIndex + " clicked!");
		// Add any additional logic you need for when a character is guessed
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
            //messageArea.append("Connected to the server.\n");
			username = JOptionPane.showInputDialog("Enter your username:");
            startListeningThread();
        } catch (IOException e) {
            //messageArea.append("Failed to connect: " + e.getMessage() + "\n");
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
					
					if(questionResponse == null){
						terminateProgram();
						return;
					}
					if(startMsg){
						questionResponse = "Good Luck!";
						startMsg = false;
					}
					
					if(line.equals("Spectating...")){
						System.out.println("yo aint playing");
						spectating = true;
					}

					if (!spectating) {
						//messageArea.append(username + questionResponse + "\n");

						String questionForOpp = JOptionPane.showInputDialog("Question for Opponent:");

					if(questionForOpp == null){
						terminateProgram();
						return;
					}
						sendMessage(username + " says: " + questionResponse + " and asks: " + questionForOpp);
					}

					
                }
            } catch (IOException e) {
                if (!terminate.get()) {
                    //messageArea.append("Lost connection to server: " + e.getMessage() + "\n");
                }
            }
        });
        listenThread.start();
    }

	private void terminateProgram() {
		terminate.set(true);
		try {
			if (theSocket != null && !theSocket.isClosed()) {
				theSocket.close();
			}
			//messageArea.append("Disconnected.\n");
		} catch (IOException e) {
		//	messageArea.append("Error closing connection: " + e.getMessage() + "\n");
		} finally {
			System.exit(0); // Ensure the application exits cleanly
		}
	}

    private void sendMessage(String message) {
        if (message.isEmpty()) {
            return; // Don't send empty messages
        }
        socketOutput.println(message);
       // messageArea.append("You: " + message + "\n");
        if (message.equalsIgnoreCase("done")) {
            terminate.set(true);
            try {
                theSocket.close();
               // messageArea.append("Disconnected.\n");
            } catch (IOException e) {
               // messageArea.append("Error closing connection: " + e.getMessage() + "\n");
            }
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(FrontEndMain::new);
		
    }
}

//if alone in the game