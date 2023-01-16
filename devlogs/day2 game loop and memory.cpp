now lets get the game loop

so in terms of input, 
the two things we care about the most is 


1.	whether a certain button is pressed down this frame 
2.	how many half transitions it went through



there is two possibilities within a tick


1.	the button went from released --> pressed --> released
			_		_
				_	

		
2.	button went from released --> pressed 
			_
				_


3.	button went from pressed --> released --> pressed 
				_
			_		_


4.	button went from pressed to released 
				_
			_



so whenever we get a state change on the input state, we compare it with the current state and if the state is different 
we increase the half transition count. depending the number of transition counts, we may special logic, for example (double clicked or double tapped);
which is why we want to keep track of double clicking







#########################################################################
############################### Memory ##################################
#########################################################################

1.		types of memory 

permenant memory:
so the permenant memory is used for information that sticks around. for example game state, entity state 

transient memory 
this is usually only used for things that are temporary. For example 
when intializing render groups, that render group is only for that one tick, that one frame.
So memory that lasts for only one frame, that is transient memory. 

debug memory 




