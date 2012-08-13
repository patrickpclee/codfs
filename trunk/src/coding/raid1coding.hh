#ifndef __RAID1CODING_HH__
#define __RAID1CODING_HH__

class Raid1Coding : public Coding {
public:
	Raid1Coding(uint32_t noOfReplications);
	~Raid1Coding();
	void display();
private:
	uint32_t _noOfReplications;
};

#endif
