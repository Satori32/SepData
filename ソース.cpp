#include <iostream>
#include <deque>
#include <cstdint>
#include <vector>
#include <memory>
#include <limits>
#include <algorithm>
#include <random>

struct SystemHeader//system land.
{
	std::uint8_t Signetuer[16] = {0,};
	std::uint8_t HeaderSize = 16 + 2 + 2;
	std::uint16_t DataSize = 0;
	std::uint8_t Continue = 0;
};

struct Data//user land.
{
	std::uint8_t Signetuer[16]={0,};
	std::uint16_t HeaderSize = 16+2+4;
	std::uint32_t DataSize = 0;
};

struct Item//what is this.
{
	SystemHeader SH;
	Data D = { 0, };

};

/** /
std::vector<Item> Write(const std::uint8_t* D, std::int32_t L) {

	if (L <= 0) { return {}; }

	Item I;
	std::vector<Item> V;
	std::uint64_t S = L + sizeof(Data) - sizeof(nullptr);

	std::size_t X = std::min<std::int32_t>(L, std::numeric_limits<std::uint16_t>::max());

	I.D.D = new std::uint8_t[X];
	I.D.Size = X;
	std::memcpy(I.D.D, (const void*)(&I.SH), I.SH.Size);


	std::memcpy(I.D.D, D, X);
	
	V.push_back(I);

	return V;
 }
/**/
std::vector<std::uint8_t> MakeVector(std::size_t L, unsigned int S = 0) {
	std::vector<std::uint8_t> R;

	std::uniform_int_distribution<> ui(0, 255);
	std::mt19937 mt(S);

	for (std::size_t i = 0; i < L; i++) {
		R.push_back(ui(mt));
	}

	return R;
}
/** /
std::vector<std::uint8_t> Read(std::deque<Item> Q) {
	Item A;
	do {
		A = Q.front();
		Q.pop_front();

		for (std::size_t i = 0; i < A.D.Size; i++) {
			std::cout << A.D.D[i] << ',';
		}
		delete[] A.D.D;
	} while (A.SH.Continue);

	std::cout << std::endl;
}
/**/
/** /
std::vector<std::uint8_t> BuildData(const std::vector<std::uint8_t>& V) {

	Data D = { "Application\n\0",16 + 2 + 4,V.size(),nullptr };
	
	std::vector<std::uint8_t> R(D.HeaderSize);

	D.DataSize = V.size();

	for (std::size_t i = 0; i < R.size(); i++) {
		switch (i)
		{
		case 0:
			for (int j = 0; j < sizeof(D.Signetuer); j++) {
				R[j] = D.Signetuer[j];
			}
			break;
		case 16:
			R[i] = (D.HeaderSize) & 0xff;
			R[i+1] = ((D.HeaderSize) &(0xff<<8))>>8;
			break;
		case 18:
			R[i]=D.DataSize &(0xff);
			R[i + 1] = (D.HeaderSize & (0xff << 8)) >> 8;
			R[i + 2] = (D.HeaderSize & (0xff << 16)) >> 16;
			R[i + 3] = (D.HeaderSize & (0xff << 24)) >> 24;
			break;
		default:
			break;
		}
	}

	R.insert(R.end(), V.begin(), V.end());

	return R;
}
/**/

std::vector<std::uint8_t> BuildData(const std::vector<std::uint8_t>& V) {
	Data D = { "Application\n\0",16 + 2 + 4,V.size() };

	std::vector<std::uint8_t> R(D.HeaderSize+16);//+16 is for alignment.
	D.DataSize = V.size();

	Data* P = reinterpret_cast<Data*>(&R[0]);

	(*P) = D;

	R.resize(D.HeaderSize);//omazinai

	R.insert(R.end(), V.begin(), V.end());

	return R;
}

bool WriteSystemHeader(SystemHeader S, std::vector<std::uint8_t>& In) {
	std::vector<std::uint8_t> R(S.HeaderSize+16);//+16 is for alignment.

	SystemHeader* P = reinterpret_cast<SystemHeader*>(&R[0]);

	(*P) = S;

	R.resize(S.HeaderSize);

	In = R;

	return true;
}


std::vector<std::vector<std::uint8_t>> Separate(const std::vector<std::uint8_t>& In) {

	std::vector<std::vector<std::uint8_t>> R;
	std::vector<std::uint8_t> V;

	SystemHeader S = { "TopHeader\n\0",16 + 2 + 2,0,1 };

	int H = 0xffff - S.HeaderSize;

	for (std::size_t i = 1; S.Continue; i++) {
		if (H * i>=In.size()) { S.Continue = 0; }
		WriteSystemHeader(S, V);
		V.insert(V.end(), In.begin() + (H * (i-1)), In.begin() + std::min(H * i, In.size()));

		R.push_back({ V.begin(),V.end() });
	}

	return R;
}

std::vector<std::uint8_t> Combin(std::vector < std::vector<std::uint8_t>>& In) {

	std::vector<std::uint8_t> R;

	auto* P = reinterpret_cast<SystemHeader*>(&(In[0][0]));

	auto H = P->HeaderSize;

	for (auto& o : In) {
		R.insert(R.end(), o.begin() + H, o.end());
	}

	return R;
}

std::vector<std::uint8_t> RipData(std::vector<std::uint8_t> In) {

	auto* P = reinterpret_cast<Data*>(&In[0]);

	auto H = P->HeaderSize;

	return { In.begin() + H,In.end() };
}

int main() {

	auto V = MakeVector(16);
	auto V2 = MakeVector(0xf);

	auto& X = V2;

	auto A = BuildData(X);

	auto B = Separate(A);

	auto C = Combin(B);

	if (A == C) { std::cout << "good!" << std::endl; }
	else { std::cout << "bad!"<<std::endl; }


	auto D = RipData(C);

	if (X == D) { std::cout << "good!" << std::endl; }
	else { std::cout << "bad!"<<std::endl; }
	
	return 0;	
}