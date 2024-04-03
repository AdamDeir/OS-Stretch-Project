import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.Socket;
import java.util.Random;
import java.util.concurrent.atomic.AtomicBoolean;
import java.awt.image.BufferedImage;
import java.awt.image.RescaleOp;



public class FrontEndMain extends JFrame {
    private Socket theSocket;
    private PrintWriter socketOutput;
    private BufferedReader socketInput;
    //private JTextArea messageArea; // For displaying messages and server responses
    private AtomicBoolean terminate = new AtomicBoolean(false); // Flag to control the listening thread
	private String username;

	private JButton[] characterButtons;
	private ImageIcon[] characterIcons;
	private boolean[] isGrayed;

	private JLabel characterDisplay;




	
    public FrontEndMain() {
        // Initialize GUI Components
		
        initializeGUI();
        String link = JOptionPane.showInputDialog("Enter the link for the game you want to join!");
        // Setup connection and start listening
        connectToServer(link, 3000);

    }


	private void initializeGUI() {
		setTitle("Network Game Console");
		setSize(800, 600); // Adjust the size to fit the game board
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	
		setLayout(new BorderLayout());
	
		initializeCharacterGrid(); // Initialize the game board

		characterDisplay = new JLabel();
		characterDisplay.setHorizontalAlignment(JLabel.CENTER);
		characterDisplay.setBorder(BorderFactory.createTitledBorder("Your Character"));
	
		// Select a random image to display as your character
		selectRandomCharacter();
	
		// Add the character display to the right side of the screen
		getContentPane().add(characterDisplay, BorderLayout.EAST);
	
		// Initialize and add other components like message area, input field, send button, etc., here

		setVisible(true);
	}

	private void selectRandomCharacter() {
    int randomIndex = new Random().nextInt(characterIcons.length);
    ImageIcon icon = characterIcons[randomIndex];
    // You can adjust the size as needed, here it's set to a width of 200 and height will be scaled automatically
    Image scaledImage = icon.getImage().getScaledInstance(200, -1, Image.SCALE_SMOOTH);
    characterDisplay.setIcon(new ImageIcon(scaledImage));
}
	
	private void initializeCharacterGrid() {
		characterButtons = new JButton[12];
		characterIcons = new ImageIcon[12];
		isGrayed = new boolean[12]; // Initialize the array, all values are false by default

		String[] characters = new String[]{
			"dounavis.jpg", "eagleson.jpg", "essex.jpg", "mcIssac.jpg",
			"ouda.jpg", "rahman.jpg", "rao.jpg", "samarabandu.jpg",
			"ali.jpg", "Yang.jpg", "Mcleod.jfif", "L'Heureux.jfif"
		};
	
		String basePath = "images/";
	
		JPanel characterGrid = new JPanel(new GridLayout(3, 4, 10, 10));
		characterGrid.setBorder(BorderFactory.createTitledBorder("Guess Who Characters"));
	
		for (int i = 0; i < characters.length; i++) {
			final int index = i;
			ImageIcon icon = new ImageIcon(basePath + characters[i]);
			Image img = icon.getImage().getScaledInstance(100, 100, Image.SCALE_SMOOTH);
			icon = new ImageIcon(img);
	
			characterButtons[i] = new JButton(icon);
			characterButtons[i].addActionListener(e -> characterButtonClicked(index));
			characterGrid.add(characterButtons[i]);
	
			characterIcons[i] = icon;
		}
	
		getContentPane().add(characterGrid, BorderLayout.CENTER);
	}
	
	
	
	
	// This method is triggered when any character button is clicked
	private void characterButtonClicked(int characterIndex) {
		BufferedImage image = new BufferedImage(
			characterIcons[characterIndex].getIconWidth(),
			characterIcons[characterIndex].getIconHeight(),
			BufferedImage.TYPE_INT_ARGB
		);
		Graphics2D g2d = image.createGraphics();
		g2d.drawImage(characterIcons[characterIndex].getImage(), 0, 0, null);
		g2d.dispose();
	
		RescaleOp rescaleOp;
		if (isGrayed[characterIndex]) {
			// If the image is grayed out, we want to "ungray" it, so we use a no-op rescale
			rescaleOp = new RescaleOp(new float[]{1f, 1f, 1f, 1f}, new float[]{0, 0, 0, 0}, null);
		} else {
			// If the image is not grayed out, we gray it out
			rescaleOp = new RescaleOp(new float[]{0.2f, 0.2f, 0.2f, 1f}, new float[]{0, 0, 0, 0}, null);
		}
	
		BufferedImage filteredImage = rescaleOp.filter(image, null);
	
		// Toggle the state
		isGrayed[characterIndex] = !isGrayed[characterIndex];
	
		// Set the new icon to the button
		characterButtons[characterIndex].setIcon(new ImageIcon(filteredImage));
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