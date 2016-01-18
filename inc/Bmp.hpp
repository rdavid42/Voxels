#ifndef BMP_HPP
# define BMP_HPP

# include <iostream>
# include <string>
# include <stdint.h>
# include <unistd.h>
# include <cstdlib>
# include <fcntl.h>

# define FILE_E1				("Error opening file")
# define HEADER_E1				("Error reading bmp header")
# define HEADER_E2				("Bad BMP header!")
# define HEADER_E3				("Error reading dib header")
# define COMPRESSION_E1			("Unsupported compression method!")
# define BPP_E1					("BMP must have 24 bpp!")
# define MALLOC_E1				("Failed to allocate BMP image")
# define DATA_E1				("Failed to read BMP data")
# define DATA_E2				("Failed to read data padding")
# define BMP_HSIZE				(14)
# define DIB_HSIZE				(40)

# define CMP_MAX				(14)

# define BI_RGB					(0)
# define BI_RLE8				(1)
# define BI_RLE4				(2)
# define BI_BITFIELDS			(3)
# define BI_JPEG				(4)
# define BI_PNG					(5)
# define BI_ALPHABITFIELDS		(6)
# define BI_CMYK				(11)
# define BI_CMYKRLE8			(12)
# define BI_CMYKRLE4			(13)

class Bmp
{
public:
	int					fd;
	unsigned char		bmp_header[BMP_HSIZE];
	uint32_t			bmp_size;
	uint32_t			data_offset;
	unsigned char		dib_header[DIB_HSIZE];
	uint32_t			width;
	uint32_t			height;
	uint32_t			bpp;
	uint32_t			compression;
	uint32_t			raw_bmp_size;
	unsigned char *		data;

	Bmp(void);
	~Bmp(void);

	int					load(char const *filename);

	Bmp &				operator=(Bmp const &rhs);
private:
	int					getBmpInfo(void);
	int					writeData(void);
	int					error(char const *s);
	int					compressionSupported();

	Bmp(Bmp const &src);
};

std::ostream			&operator<<(std::ostream &o, Bmp const &i);

#endif