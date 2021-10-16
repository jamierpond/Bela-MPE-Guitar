/***** BinaryMatrix.h *****/



// So this is a data structure that describes a naieve touch. 
// A touch as it is instantaneously described by Trill,
// irrespective of its persistence. 
struct Touch
{
	bool isActive;
	int velocity;
	float touchLocation;
	float initialTouchLocation;
};

class TouchMatrix
{
public:
	TouchMatrix(int w, int h)
	{
		matrix.resize(w);
		for(int i = 0; i < matrix.size(); i++)
		{
			matrix[i].resize(h);
		}
	}
	
	void reset(bool valueToResetTo = false)
	{
		for(int w = 0; w < matrix.size(); w++)
		{
			for(int h = 0; h < matrix[0].size(); h++)
			{
				matrix[w][h].isActive = valueToResetTo;
				matrix[w][h].velocity = 0;
			}
		}
	}
	
	void printMatrix()
	{
		for(int w = 0; w < matrix.size(); w++)
		{
			for(int h = 0; h < matrix[0].size(); h++)
			{
				rt_printf("%i   ", (int)matrix[w][h].isActive);
			}
			rt_printf("\n");
		}
	}
	
	void setValue(int w, int h, bool value, int velocity, float touchLocation)
	{
		matrix[w][h].isActive = value;
		matrix[w][h].velocity = velocity;
		matrix[w][h].touchLocation = touchLocation;
	}

	// This should use neon at some point...
	void copy(TouchMatrix& matrixToCopy)
	{
		for(int w = 0; w < matrix.size(); w++)
		{
			for(int h = 0; h < matrix[0].size(); h++)
			{
				auto mtc = matrixToCopy.getValue(w, h);
				setValue(w, h, mtc.isActive, mtc.velocity, mtc.touchLocation);
			}
		}
	}
	
	Touch getValue(int w, int h)
	{
		return matrix[w][h];
	}
	
private:
	std::vector<std::vector<Touch>> matrix;
};





