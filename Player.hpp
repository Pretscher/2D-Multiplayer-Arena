#pragma once

class Player {
public:
	Player(int i_x, int i_y, float i_vel);

private:
	int x;
	int y;
	float velocity;





    //Getters and Setters (autogenerated)
public:
    inline int getX() { return x; }
    inline void setX(int x) { this->x = x; }

    inline int getY() { return y; }
    inline void setY(int y) { this->y = y; }

    inline float getVelocity() { return velocity; }
    inline void setVelocity(float velocity) { this->velocity = velocity; }

};
