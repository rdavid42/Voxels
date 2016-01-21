
#include "Bmp.hpp"

Bmp::Bmp(void)
{
	this->data = 0;
	return ;
}

Bmp::~Bmp(void)
{
	if (this->data != 0)
		delete [] this->data;
	return ;
}

int
Bmp::error(char const *s)
{
	std::cerr << s << std::endl;
	return (0);
}

inline static int
deserialize(unsigned char const *s, int o, int size)
{
	int			i;
	int			r;

	r = 0;
	for (i = 0; i < size; ++i)
		r |= s[o + i] << (i << 3);
	return (r);
}

int
Bmp::compressionSupported()
{
	static int const		support[CMP_MAX] = { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
/*	static char const		*msg[CMP_MAX] =
	{
		"BI_RGB",
		"BI_RLE8",
		"BI_RLE4",
		"BI_BITFIELDS",
		"BI_JPEG",
		"BI_PNG",
		"UNKNOWN_1",
		"UNKNOWN_2",
		"UNKNOWN_3",
		"UNKNOWN_4",
		"BI_ALPHABITFIELDS",
		"BI_CMYK",
		"BI_CMYKRLE8",
		"BI_CMYKRLE4"
	};*/

	std::cerr << compression << std::endl;
/*	std::cerr << msg[compression] << " compression method ";
	if (support[compression])
		std::cerr << "is supported";
	else
		std::cerr << "isn't supported";
	std::cerr << std::endl;*/
	return (support[compression]);
}

int
Bmp::getBmpInfo(void)
{
	int					unused_header_size;
	unsigned char		*unused_header;

	if (bmp_header[0] != 'B' && bmp_header[1] != 'M')
		return (error(HEADER_E2));
	bmp_size = deserialize(bmp_header, 2, 4);
	// std::cerr << "bmp_size: " << bmp_size << std::endl;
	data_offset = deserialize(bmp_header, 10, 4);
	// std::cerr << "data_offset: " << data_offset << std::endl;
	width = deserialize(dib_header, 4, 4);
	// std::cerr << "width: " << width << std::endl;
	height = deserialize(dib_header, 8, 4);
	// std::cerr << "height: " << height << std::endl;
	bpp = deserialize(dib_header, 14, 2);
	// std::cerr << "bpp: " << bpp << std::endl;
	compression = deserialize(dib_header, 16, 4);
	if (!compressionSupported())
		return (error(COMPRESSION_E1));
	raw_bmp_size = deserialize(dib_header, 20, 4);
	// std::cerr << "raw_bmp_size: " << raw_bmp_size << std::endl;
	unused_header_size = data_offset - (BMP_HSIZE + DIB_HSIZE);
	if (!raw_bmp_size)
		raw_bmp_size = bmp_size - (BMP_HSIZE + DIB_HSIZE);
	// std::cerr << "unused header size: " << unused_header_size << std::endl;
	if (unused_header_size > 0)
	{
		unused_header = new unsigned char[unused_header_size];
		(void)!read(fd, unused_header, unused_header_size);
		delete [] unused_header;
	}
	return (1);
}

int
Bmp::writeData(void)
{
	unsigned char		buf[4];
	uint32_t			i;
	uint32_t			j;
	uint32_t			k;

	if (bpp == 24)
	{
		k = 0;
		i = 0;
		while (i < height)
		{
			j = 0;
			while (j < width)
			{
				if (read(fd, buf, 3) == -1)
					return (0);
				data[k++] = buf[2];
				data[k++] = buf[1];
				data[k++] = buf[0];
				j++;
			}
			if (j % 4)
				if (read(fd, buf, 1) == -1)
					return (0);
			i++;
		}
	}
	else if (bpp == 32)
	{
		k = 0;
		i = 0;
		while (i < height)
		{
			j = 0;
			while (j < width)
			{
				if (read(fd, buf, 4) == -1)
					return (0);
				// std::cerr << "Pixel R:" << int(buf[1]) << ", G:" << int(buf[2]) << ", B:" << int(buf[3]) << ", A:" << int(buf[0]) << std::endl;
				data[k++] = buf[1];
				data[k++] = buf[2];
				data[k++] = buf[3];
				data[k++] = buf[0];
				j++;
			}
			i++;
		}
	}
	return (1);
}

int
Bmp::load(char const *filename)
{
	// std::cerr << "Loading " << filename << std::endl;
	if ((fd = open(filename, O_RDONLY)) == -1)
		return (error(FILE_E1));
	if (read(fd, bmp_header, BMP_HSIZE) == -1)
		return (error(HEADER_E1));
	if (read(fd, dib_header, DIB_HSIZE) == -1)
		return (error(HEADER_E3));
	if (!getBmpInfo())
		return (0);
	data = new unsigned char[raw_bmp_size];
	if (!writeData())
		return (0);
	close(fd);
	return (1);
}

Bmp &
Bmp::operator=(Bmp const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream &
operator<<(std::ostream &o, Bmp const &i)
{
	o	<< "Bmp: " << &i;
	return (o);
}
