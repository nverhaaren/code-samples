import javax.swing.*;
import physics.*;
import level.*;
import java.util.*;
import java.awt.event.*;
//import java.awt.*;

public class GameRunner extends JFrame implements KeyListener
{
	// Because JFrame is Serializable (ignore if you don't know what that means)
	private static final long serialVersionUID = 1L;
	
	// The thing we draw on
	private SimulatorCanvas canvas;
	
	// The simulator
	private Environment sim;
	private ArrayList<Level> levels = new ArrayList<Level>();
	
	// Used to stabilize how often we paint
	private long timeOfLastUpdate;
	
	// way to use the 'Esc' key like in demo (to exit)
	private boolean running;
	
	// prevent the cannon shooting in the middle of a step, possibly causing problems
	private boolean fire;
	
	public static void main( String args[] )
	{
		// Create a window that causes the program to exit when it closes
		GameRunner frame = new GameRunner( "Mildly Perturbed Magnets" );
		frame.setDefaultCloseOperation( EXIT_ON_CLOSE );
		frame.go();
	}
	
	private void go()
	{
		// Do various graphics things
		addComponentsToFrame();
		pack();
		setResizable( false );
		setVisible( true );
		
		// Do various keyboard things
		setFocusTraversalKeysEnabled(false);
		addKeyListener( this );
		
		// For debugging purposes
		//System.out.println( "Text" );
		
		while( running )
		{
			if ( System.currentTimeMillis() - timeOfLastUpdate >= 10 )
			{
				//System.out.println( System.currentTimeMillis() - frame.timeOfLastUpdate );
				timeOfLastUpdate = System.currentTimeMillis();
				
				if ( canvas.started() )
				{
					if ( fire )
					{
						canvas.getCurrentLevel().getCannon().fire();
						fire = false;
					}
					
					canvas.getCurrentLevel().step();
					
					canvas.repaint(1L);
					
				}
				
				// This is a useful debugging tool
				//System.out.println( frame.sim.getParticles().get(0) );
			}
		}
		
		//System.out.println("We've reached the end of main!");
		System.exit(0);
	}
	
	public GameRunner( String name )
	{
		super( name );
		
		running = true;
		fire = false;
		
		// create the simulator and the things to put into it.
		sim = new Environment( 0.01, new Vector2D(700.0, 500.0) );
		
		initLevels();
		
		canvas = new SimulatorCanvas( sim, levels );
		
		timeOfLastUpdate = System.currentTimeMillis();
	}
	
	private void initLevels()
	{
		/* Here we will create levels. First, all the Walls, the Cannon, the Antimatter, and the Fields are created
		 * and correctly initialized. Then, a Level is created that has all these things. The Level is added to the 
		 * ArrayList levels, which will later be used, when complete, as  the collection of all the levels in the game.
		 * --Nathaniel Verhaaren, May 14, 2012
		 */
		
		sim = new Environment( 0.01, new Vector2D( 700.0, 500.0 ) );
		
		//----------------------LEVEL 1--------------------------------
		
		Level level1 = new Level( sim, new Vector2D( 20.0, 250.0 ), new Vector2D( 600.0, 400.0 ),
								  Math.PI / 2, -Math.PI / 2, 5, 4 );
		
		level1.addPathPoint(new Vector2D( 690.0, 250.0), 20.0);
		level1.addPathPoint(new Vector2D(690.0,100.0), 50.0);
		level1.addPathPoint(new Vector2D(690.0,0.0), 5.0);
		
		Antimatter l1_a1 = new Antimatter( new Vector2D( 695.0, 0.0 ), new Vector2D( 700.0, 500.0 ) );
		level1.addWall( l1_a1 );
		Wall l1_w1 = new Wall( new Vector2D( 0.0, 0.0 ), new Vector2D( 5.0, 500.0 ) );
		level1.addWall( l1_w1 );
		Wall l1_w2 = new Wall( new Vector2D( 5.0, 0.0 ), new Vector2D( 695.0, 5.0 ) );
		level1.addWall(l1_w2);
		Wall l1_w3 = new Wall( new Vector2D( 5.0, 495.0 ), new Vector2D( 695.0, 500.0) );
		level1.addWall(l1_w3);
		
		Wall l1_w4 = new Wall( new Vector2D( 345.0, 150.0 ), new Vector2D( 355.0, 300.0 ) );
		l1_w4.bounceY(10.0, 310.0, -30.0);
		level1.addWall(l1_w4);
		
		ElectricField l1_ef1 = new ElectricField( new Vector2D( 10.0, 0.0 ),
												  new Vector2D( 0.0, 0.0 ),
												  new Vector2D( 700.0, 500.0 ) );
		MagneticField l1_mf1 = new MagneticField( 1.0,
												  new Vector2D( 0.0, 0.0 ),
												  new Vector2D( 700.0, 500.0 ) );
		
		level1.addField( l1_ef1 );
		level1.addField( l1_mf1 );
		
		levels.add(level1);
		
		//---------------------END LEVEL 1---------------------------
		//-------------------------LEVEL 2---------------------------
		
		Level level2 = new Level( sim, new Vector2D( 20.0, 250.0 ), new Vector2D( 600.0, 400.0 ),
				  Math.PI / 2, -Math.PI / 2, 5, 4 );

		level2.addPathPoint(new Vector2D( 550.0, 250.0), 10.0);
		level2.addPathPoint(new Vector2D(550.0,100.0), 10.0);
		level2.addPathPoint(new Vector2D(550.0,0.0), 5.0);
		
		Antimatter l2_a1 = new Antimatter( new Vector2D( 695.0, 0.0 ), new Vector2D( 700.0, 500.0 ) );
		level2.addWall( l2_a1 );
		Antimatter l2_a2 = new Antimatter( new Vector2D( 560.0, 200.0 ), new Vector2D( 620.0, 220.0 ) );
		level2.addWall( l2_a2 );
		Antimatter l2_a3 = new Antimatter( new Vector2D( 560.0, 250.0 ), new Vector2D( 620.0, 270.0 ) );
		level2.addWall( l2_a3 );
		Antimatter l2_a4 = new Antimatter( new Vector2D( 560.0, 300.0 ), new Vector2D( 620.0, 320.0 ) );
		level2.addWall( l2_a4 );
		Antimatter l2_a5 = new Antimatter( new Vector2D( 560.0, 350.0 ), new Vector2D( 620.0, 370.0 ) );
		level2.addWall( l2_a5 );
		Wall l2_w1 = new Wall( new Vector2D( 0.0, 0.0 ), new Vector2D( 5.0, 500.0 ) );
		level2.addWall( l2_w1 );
		Wall l2_w2 = new Wall( new Vector2D( 5.0, 0.0 ), new Vector2D( 695.0, 5.0 ) );
		level2.addWall(l2_w2);
		Wall l2_w3 = new Wall( new Vector2D( 5.0, 495.0 ), new Vector2D( 695.0, 500.0) );
		level2.addWall(l2_w3);
		Wall l2_w4 = new Wall( new Vector2D( 200.0, 300.0 ), new Vector2D( 500.0, 500.0) );
		level2.addWall(l2_w4);
		Wall l2_w5 = new Wall( new Vector2D( 200.0, 0.0 ), new Vector2D( 500.0, 200.0) );
		level2.addWall(l2_w5);
		
		ElectricField l2_ef1 = new ElectricField( new Vector2D( 10, 0.0 ),
								  new Vector2D( 10.0, 10.0 ),
								  new Vector2D( 600.0, 300.0 ) );
		
		
		level2.addField( l2_ef1 );
		
		
		level2.addPathPoint( new Vector2D( 600.0, 0.0 ), 10.0 );
		
		levels.add(level2);
		
		//------------------------END LEVEL 2--------------------------------
		//----------------------------LEVEL 3--------------------------------
		
		Level level3 = new Level( sim, new Vector2D( 20.0, 250.0 ), new Vector2D( 600.0, 400.0 ),
				  Math.PI / 2, -Math.PI / 2, 5, 4 );

		level3.addPathPoint(new Vector2D( 500.0, 250.0), 10.0);
		level3.addPathPoint(new Vector2D(500.0,100.0), 10.0);
		level3.addPathPoint(new Vector2D(500.0,0.0), 10.0);
		
		
		Antimatter l3_a1 = new Antimatter( new Vector2D( 520.0, 200.0 ), new Vector2D( 530.0, 230.0 ) );
		level3.addWall( l3_a1 );
		Antimatter l3_a2 = new Antimatter( new Vector2D( 520.0, 260.0 ), new Vector2D( 530.0, 290.0 ) );
		level3.addWall( l3_a2 );
		Antimatter l3_a3 = new Antimatter( new Vector2D( 520.0, 320.0 ), new Vector2D( 530.0, 350.0 ) );
		level3.addWall( l3_a3 );
		Antimatter l3_a4 = new Antimatter( new Vector2D( 520.0, 350.0 ), new Vector2D( 530.0, 380.0 ) );
		level3.addWall( l3_a4 );
		
		Wall l3_w1 = new Wall( new Vector2D( 0.0, 0.0 ), new Vector2D( 5.0, 500.0 ) );
		level3.addWall( l3_w1 );
		Wall l3_w2 = new Wall( new Vector2D( 5.0, 0.0 ), new Vector2D( 695.0, 5.0 ) );
		level3.addWall(l3_w2);
		Wall l3_w3 = new Wall( new Vector2D( 5.0, 495.0 ), new Vector2D( 695.0, 500.0) );
		level3.addWall(l3_w3);
		Wall l3_w4 = new Wall( new Vector2D( 250.0, 200.0 ), new Vector2D( 450.0, 500.0) );
		level3.addWall(l3_w4);
		
		
		ElectricField l3_ef1 = new ElectricField( new Vector2D( 10.0, 10.0 ),
										  new Vector2D( 0.0, 0.0 ),
										  new Vector2D( 450.0, 200.0 ) );
		MagneticField l3_mf1 = new MagneticField( 1.0,
										  new Vector2D( 0.0, 0.0 ),
										  new Vector2D( 250.0, 500.0 ) );
		
		level3.addField( l3_ef1 );
		level3.addField( l3_mf1 );
		
		level3.addPathPoint( new Vector2D( 700.0, 0.0 ), 10.0 );
		
		levels.add(level3);
		
		//--------------------------END LEVEL 3--------------------------------
	}
	
	public void addComponentsToFrame()
	{
		this.add( canvas );
	}
	
	// I didn't have to override this, but now I can't forget setIsDisplayed()
	public void setVisible( boolean b )
	{
		super.setVisible( b );
		canvas.setIsDisplayed( b );
	}
	
	// The following three methods must be overridden to be a KeyListener
	public void keyPressed(KeyEvent e)
	{
		if ( e.getKeyCode() == KeyEvent.VK_ESCAPE )
		{
			running = false;
			return;
		}
		
		if ( !canvas.started() )
		{
			if ( e.getKeyCode() == KeyEvent.VK_ENTER )
				canvas.start();
			return;
		}
		
		switch( e.getKeyCode() ) 
		{
			case KeyEvent.VK_RIGHT:
				canvas.getCurrentLevel().getCannon().incSpeed();
				break;
			case KeyEvent.VK_LEFT:
				canvas.getCurrentLevel().getCannon().decSpeed();
				break;
			//remember that the y-axis is flipped!
			case KeyEvent.VK_UP:
				canvas.getCurrentLevel().getCannon().decAngle();
				break;
			case KeyEvent.VK_DOWN:
				canvas.getCurrentLevel().getCannon().incAngle();
				break;
			case KeyEvent.VK_ENTER:
				if ( canvas.getCurrentLevel().finished() && !canvas.finished() )
					canvas.nextLevel();
				break;
			default:
				//System.out.println("Pressed: " + KeyEvent.getKeyText(e.getKeyCode() ));
				e.consume();
		}
	}
	
	public void keyReleased(KeyEvent e)
	{
		int keyCode = e.getKeyCode();
		
		if ( keyCode == KeyEvent.VK_SPACE && canvas.getCurrentLevel().started() )
		{
			fire = true;
			canvas.shotCount++;
		}
		//System.out.println("Released: " + KeyEvent.getKeyText(keyCode));
		e.consume();
	}
	
	public void keyTyped(KeyEvent e)
	{
	}
}
