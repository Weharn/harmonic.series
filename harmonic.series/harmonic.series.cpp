#include <iostream>
#include <vector>
#include <fstream>
#include <string>

const long bitrate{ 44100 };
const int bitdepth{ 16 };
const float pi{ 3.1416f };

struct Note
{
	float freq{};
	std::string note{};
};

class Harmonics
{
/* WIP; I think I'll store the list of all notes in a .txt file, then read them in when required to coordinate a given note to
* the relevant frequency, then free the memory. Then the maths to find the harmonic series should be quite simple, then I can 
* feed that into some form of pattern class which will store the series and have a process to create the waveform vector.
* then I'll just write that to file and Robbie's my uncle. */
public:

};

class Sine
{
private:

	float m_angle{};
	float m_step{};

public:

	std::vector<float> m_amplitude{};
	std::vector<float> m_sinevec{};
	int m_postwr_pos{};
	float m_frequency{};
	float m_duration{};

	void init(std::vector<float> ampl, float freq, float durn)
	{
		m_step = sin(2 * pi * (m_frequency / bitrate));
		m_amplitude = ampl;
		m_frequency = freq;
		m_duration = durn;
	}

	void init(float durn)
	{
		m_step = sin(2 * pi * (m_frequency / bitrate));
		m_duration = durn;
	}

	float gen_sine()
	{
		float sample{ sin(m_angle) };

		m_angle += m_step;

		return sample;
	}

	void sine_vec()
	{
		float cycles{ m_duration * bitrate };

		for (int i{}; i < cycles; ++i)
		{
			m_sinevec.push_back(gen_sine());
		}
	}

	void write(std::ofstream& ofs)
	{
		auto amplmod{ (pow(2, (bitdepth - 1)) - 1) };

		for (int i{}; i < (bitrate * m_duration); ++i)
		{
			auto fsample{ m_sinevec[i] * amplmod };
			int isample{ static_cast<int>(fsample) };

			ofs.write((reinterpret_cast<char*>(&isample)), 2);
		}

		m_postwr_pos = ofs.tellp();
	}
};

class Master
{
public:

	int m_postwr_pos{};
	std::vector<float> m_master_vec{};

	void add(Sine& wave)															//adds a Sine object to m_master_vec
	{
		for (int i{}; i < wave.m_sinevec.size(); ++i)
		{
			m_master_vec.push_back(wave.m_sinevec[i]);
		}
	}

	void merge(Master& master1, Master& master2)													//merges two Master objects into m_master_vec
	{
		int i{};
		int larger_c{};

		if (master1.m_master_vec.size() < master2.m_master_vec.size())
		{
			larger_c = 0;
		}
		else if (master1.m_master_vec.size() > master2.m_master_vec.size())
		{
			larger_c = 1;
		}
		else
		{
			larger_c = 2;
		}

		size_t m1size{ master1.m_master_vec.size() };
		size_t m2size{ master2.m_master_vec.size() };

		if (larger_c == 0)
		{
			while (i < m1size)
			{
				m_master_vec.push_back((master1.m_master_vec[i] + master2.m_master_vec[i]) * 0.5);

				++i;
			}

			while (i < m2size)
			{
				m_master_vec.push_back(master2.m_master_vec[i]);

				++i;
			}

		}
		else if (larger_c == 1)
		{
			while (i < m2size)
			{
				m_master_vec.push_back((master1.m_master_vec[i] + master2.m_master_vec[i]) * 0.5);

				++i;
			}

			while (i < m1size)
			{
				m_master_vec.push_back(master1.m_master_vec[i]);

				++i;
			}
		}
		else
		{
			for (int i{}; i < m1size; ++i)
			{
				m_master_vec.push_back((master1.m_master_vec[i] + master2.m_master_vec[i]) * 0.5);
			}
		}
	}

	void write(std::ofstream& ofs)
	{
		auto amplmod{ pow(2, (bitdepth - 1)) - 1 };
		int mvec_size{ static_cast<int>(m_master_vec.size()) };

		for (int i{}; i < mvec_size; ++i)
		{
			auto fsample{ m_master_vec[i] * amplmod };
			int isample{ static_cast<int>(fsample) };

			ofs.write((reinterpret_cast<char*>(&isample)), 2);
		}

		m_postwr_pos = ofs.tellp();
	}
};

class Header
{
public:
	int m_preaudiop{};
	std::ofstream& ofs;

	Header(std::ofstream& tmpofs) : ofs{ tmpofs }
	{
	}

	template <typename T>
	void writeofs(std::ofstream& ofs, T value, int size)
	{
		ofs.write(reinterpret_cast<const char*> (&value), size);
	}

	void header_i()
	{
		//Header chunk
		ofs << "RIFF";
		ofs << "----";
		ofs << "WAVE";

		// Format chunk
		ofs << "fmt ";
		writeofs(ofs, 16, 4); // Size
		writeofs(ofs, 1, 2); // Compression code
		writeofs(ofs, 1, 2); // Number of channels
		writeofs(ofs, bitrate, 4); // Sample rate
		writeofs(ofs, bitrate * bitdepth / 8, 4); // Byte rate
		writeofs(ofs, bitdepth / 8, 2); // Block align
		writeofs(ofs, bitdepth, 2); // Bit depth

		//Data chunk
		ofs << "data";
		ofs << "----";

		m_preaudiop = ofs.tellp();
	}

	void header_c(int postaudiop)
	{
		ofs.seekp(m_preaudiop - 4);
		writeofs(ofs, postaudiop - m_preaudiop, 4);

		ofs.seekp(4, std::ios::beg);
		writeofs(ofs, postaudiop - 8, 4);
	}

};

int main()
{
    std::cout << "Hello World!\n";
}