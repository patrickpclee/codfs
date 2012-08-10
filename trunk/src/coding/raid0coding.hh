#ifndef __RAID0CODING_HH__
#define __RAID0CODING_HH__

class Raid0Coding : public Coding {
public:
	Raid0Coding(uint32_t noOfStrips);
	~Raid0Coding();
	void display();
private:
	uint32_t _noOfStrips;
};

#endif
