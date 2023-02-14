//* =+= Main File =+= *//

#include <fstream>
#include "Image.h"
using namespace std;

/* === Clamp Function: Keeps the pixels in range === */
unsigned char normalize(int i)
{
    if(i > 255)
        return 255;
    if(i < 0)
        return 0;

    return i;
}

Image loadImage(string fileName)
{
    ifstream inputFile(fileName, ios_base::binary);

    Header header;
    Image newImage;

    /* == Opposite of next function == */
    inputFile.read((char*) &header.idLength, sizeof(header.idLength));
    inputFile.read((char*) &header.colorMapType, sizeof(header.colorMapType));
    inputFile.read((char*) &header.dataTypeCode, sizeof(header.dataTypeCode));
    inputFile.read((char*) &header.colorMapOrigin, sizeof(header.colorMapOrigin));
    inputFile.read((char*) &header.colorMapLength, sizeof(header.colorMapLength));
    inputFile.read((char*) &header.colorMapDepth, sizeof(header.colorMapDepth));

    inputFile.read((char*) &header.xOrigin, sizeof(header.xOrigin));
    inputFile.read((char*) &header.yOrigin, sizeof(header.yOrigin));
    inputFile.read((char*) &header.width, sizeof(header.width));
    inputFile.read((char*) &header.height, sizeof(header.height));
    inputFile.read((char*) &header.bitsPerPixel, sizeof(header.bitsPerPixel));
    inputFile.read((char*) &header.imageDescriptor, sizeof(header.imageDescriptor));

    unsigned int area = (header.width)*(header.height);

    newImage = Image(header);

    //Read the pixel color channels last, instead of first:

    for(unsigned int i = 0; i < area; i++) {
        Pixel placeholder;
        //read to set the value for the placeholder Pixels.
        inputFile.read((char*)&placeholder.blue, sizeof(placeholder.blue));
        inputFile.read((char*)&placeholder.green, sizeof(placeholder.green));
        inputFile.read((char*)&placeholder.red, sizeof(placeholder.red));

        //Now set the actual pixels at i to those values:
        newImage.pixelVector.at(i).blue = placeholder.blue;
        newImage.pixelVector.at(i).green = placeholder.green;
        newImage.pixelVector.at(i).red = placeholder.red;
    }

    inputFile.close();

    return newImage;
}

void outputImage(string fileName, const Image& outImage)
{
    //By setting as "output/" we make it so that the file is accessible no matter the
    //directory, as long as there is an output folder!

    ofstream outputFile("output/" + fileName, ios_base::binary);

    /* == Write into the outputFile the headerObject's variables, and the sizes of those variables! == */
    outputFile.write((char*) &outImage.headerObject.idLength, sizeof(outImage.headerObject.idLength));
    outputFile.write((char*) &outImage.headerObject.colorMapType, sizeof(outImage.headerObject.colorMapType));
    outputFile.write((char*) &outImage.headerObject.dataTypeCode, sizeof(outImage.headerObject.dataTypeCode));
    outputFile.write((char*) &outImage.headerObject.colorMapOrigin, sizeof(outImage.headerObject.colorMapOrigin));
    outputFile.write((char*) &outImage.headerObject.colorMapLength, sizeof(outImage.headerObject.colorMapLength));
    outputFile.write((char*) &outImage.headerObject.colorMapDepth, sizeof(outImage.headerObject.colorMapDepth));

    outputFile.write((char*) &outImage.headerObject.xOrigin, sizeof(outImage.headerObject.xOrigin));
    outputFile.write((char*) &outImage.headerObject.yOrigin, sizeof(outImage.headerObject.yOrigin));
    outputFile.write((char*) &outImage.headerObject.width, sizeof(outImage.headerObject.width));
    outputFile.write((char*) &outImage.headerObject.height, sizeof(outImage.headerObject.height));
    outputFile.write((char*) &outImage.headerObject.bitsPerPixel, sizeof(outImage.headerObject.bitsPerPixel));
    outputFile.write((char*) &outImage.headerObject.imageDescriptor, sizeof(outImage.headerObject.imageDescriptor));

    //Set up the pixel color channels first:
    for(unsigned int i = 0; i < outImage.pixelVector.size(); i++) {
        //cout << "Writing pixel #" << i << " / " << outImage.pixelVector.size() << endl;
        Pixel pixel = outImage.pixelVector.at(i);
        outputFile.write((char *) &pixel.blue, 1);
        outputFile.write((char *) &pixel.green, 1);
        outputFile.write((char *) &pixel.red, 1);
    }
    outputFile.close();
}

/* === Multiply two image layers together === */
Image Multiply(const Image &layerOne, const Image &layerTwo)
{
    Image newImage(layerOne.headerObject);

    //Don't forget about rounding to normalize values:
    for(unsigned int i = 0; i < layerOne.pixelVector.size(); i ++)
    {
        Pixel newPixel;

        //To keep the range, we divide by the max.
        newPixel.blue = (unsigned char)(round((layerOne.pixelVector.at(i).blue * layerTwo.pixelVector.at(i).blue)/255.0));
        newPixel.green = (unsigned char)(round((layerOne.pixelVector.at(i).green * layerTwo.pixelVector.at(i).green)/255.0));
        newPixel.red = (unsigned char)(round((layerOne.pixelVector.at(i).red * layerTwo.pixelVector.at(i).red)/255.0));


        newImage.pixelVector.at(i) = newPixel;
    }
    return newImage;
}

/* === Subtract top layer from bottom layer === */
Image Subtract(const Image &layerOne, const Image &layerTwo)
{
    Image newImage(layerOne.headerObject);

    for(unsigned int i = 0; i < layerTwo.pixelVector.size(); i ++)
    {
        Pixel newPixel;

        newPixel.blue = (unsigned char)normalize(layerTwo.pixelVector.at(i).blue - layerOne.pixelVector.at(i).blue);
        newPixel.green = (unsigned char)normalize(layerTwo.pixelVector.at(i).green - layerOne.pixelVector.at(i).green);
        newPixel.red = (unsigned char)normalize(layerTwo.pixelVector.at(i).red - layerOne.pixelVector.at(i).red);

        // Now set the values of the layer's pixel to those values AFTER
        // the check:
        newImage.pixelVector.at(i) = newPixel;
    }
    return newImage;
}

/* === Screen blending mode === */
// C = 1 - (1-A)*(1-B) oder (1-C) = (1-A)*(1-B)
Image Screen(const Image &layerOne, const Image &layerTwo)
{
    Image newImage(layerOne.headerObject);

    //Don't forget about rounding to normalize values:
    for(unsigned int i = 0; i < layerOne.pixelVector.size(); i ++)
    {
        Pixel newPixel;

        // C = 255 - (255-A)*(255-B)
        //To keep the range, we divide by the max.
        newPixel.blue = normalize(round((1.0 - (1.0 - (float)layerOne.pixelVector.at(i).blue / 255.0) * (1.0 - (float)layerTwo.pixelVector.at(i).blue / 255.0)) * 255.0));
        newPixel.green = normalize(round((1.0 - (1.0 - (float)layerOne.pixelVector.at(i).green / 255.0) * (1.0 - (float)layerTwo.pixelVector.at(i).green / 255.0)) * 255.0));
        newPixel.red = normalize(round((1.0 - (1.0 - (float)layerOne.pixelVector.at(i).red / 255.0) * (1.0 - (float)layerTwo.pixelVector.at(i).red / 255.0)) * 255.0));

        newImage.pixelVector.at(i) = newPixel;
    }

    return newImage;
}

/* === Overlay blending mode === */
Image Overlay(const Image &layerOne, const Image &layerTwo)
{
    Image newImage(layerOne.headerObject);
    for(unsigned int i = 0; i < layerOne.pixelVector.size(); i++)
    {
        Pixel newPixel;
        if (newImage.pixelVector.at(i).red <= 0.5)
        {
            newPixel.red = normalize(2.0 * round(layerOne.pixelVector.at(i).red * layerTwo.pixelVector.at(i).red));
        } else if (newImage.pixelVector.at(i).red > 0.5)
        {
            newPixel.red = normalize(round((1.0 - (2.0 * ((1.0 - (float)layerOne.pixelVector.at(i).red / 255.0) * (1.0 - (float)layerTwo.pixelVector.at(i).red / 255.0))) * 255.0)));
        }

        //
        if (newImage.pixelVector.at(i).blue <= 0.5)
        {
            newPixel.blue = normalize(2.0 * round(layerOne.pixelVector.at(i).blue * layerTwo.pixelVector.at(i).blue));
        } else if (newImage.pixelVector.at(i).blue > 0.5)
        {
            newPixel.blue = normalize(round((1.0 - (2.0 * ((1.0 - (float)layerOne.pixelVector.at(i).blue / 255.0) * (1.0 - (float)layerTwo.pixelVector.at(i).blue / 255.0))) * 255.0)));
        }

        //
        if (newImage.pixelVector.at(i).green <= 0.5)
        {
            newPixel.green = normalize(2.0 * round(layerOne.pixelVector.at(i).green * layerTwo.pixelVector.at(i).green));
        } else if (newImage.pixelVector.at(i).green > 0.5)
        {
            newPixel.green = normalize(round((1.0 - (2.0 * ((1.0 - (float)layerOne.pixelVector.at(i).green / 255.0) * (1.0 - (float)layerTwo.pixelVector.at(i).green / 255.0))) * 255.0)));
        }

        newImage.pixelVector.at(i) = newPixel;

    }
    return newImage;
}

/* === Add to the channels of an image's pixels === */
Image addChannel(Image &layerOne, const string& colorChannel, int amount)
{
    Image newImage(layerOne.headerObject);

    if(colorChannel == "blue")
    {
        for(unsigned int i = 0; i < layerOne.pixelVector.size(); i++)
        {
            newImage.pixelVector.at(i).blue = normalize((int)layerOne.pixelVector.at(i).blue + amount);
            newImage.pixelVector.at(i).green = layerOne.pixelVector.at(i).green;
            newImage.pixelVector.at(i).red = layerOne.pixelVector.at(i).red;
        }
    }
    if(colorChannel == "green")
    {
        for(unsigned int i = 0; i < layerOne.pixelVector.size(); i++)
        {
            newImage.pixelVector.at(i).green = normalize((int)layerOne.pixelVector.at(i).green + amount);
            newImage.pixelVector.at(i).red = layerOne.pixelVector.at(i).red;
            newImage.pixelVector.at(i).blue = layerOne.pixelVector.at(i).blue;
        }
    }
    if(colorChannel == "red")
    {
        for(unsigned int i = 0; i < layerOne.pixelVector.size(); i++)
        {
            newImage.pixelVector.at(i).red = normalize((int)layerOne.pixelVector.at(i).red + amount);
            newImage.pixelVector.at(i).green = layerOne.pixelVector.at(i).green;
            newImage.pixelVector.at(i).blue = layerOne.pixelVector.at(i).blue;
        }
    }
    return newImage;
}

/* === Scale the channels of an image's pixels === */
Image scaleChannel(Image &layerOne, const string& colorChannel, int amount)
{
    Image newImage(layerOne.headerObject);



    if(colorChannel == "blue")
    {
        for(unsigned int i = 0; i < layerOne.pixelVector.size(); i++)
        {
            newImage.pixelVector.at(i).blue = normalize((int)layerOne.pixelVector.at(i).blue * amount);
            newImage.pixelVector.at(i).green = layerOne.pixelVector.at(i).green;
            newImage.pixelVector.at(i).red = layerOne.pixelVector.at(i).red;

        }
    }
    if(colorChannel == "green")
    {
        for(unsigned int i = 0; i < layerOne.pixelVector.size(); i++)
        {
            newImage.pixelVector.at(i).green = normalize((int)layerOne.pixelVector.at(i).green * amount);
            newImage.pixelVector.at(i).red = layerOne.pixelVector.at(i).red;
            newImage.pixelVector.at(i).blue = layerOne.pixelVector.at(i).blue;

        }
    }
    if(colorChannel == "red")
    {
        for(unsigned int i = 0; i < layerOne.pixelVector.size(); i++)
        {
            newImage.pixelVector.at(i).red = normalize((int)layerOne.pixelVector.at(i).red * amount);
            newImage.pixelVector.at(i).green = layerOne.pixelVector.at(i).green;
            newImage.pixelVector.at(i).blue = layerOne.pixelVector.at(i).blue;

        }
    }
    return newImage;
}


/* === 180 Rotation mode === */
//Image Rotation(Image &original, Image &originalCopy)
/* === 180 Rotation mode === */

//void Rotation(Image &textImg, Image &newImg) {
//make a new num to store the current image then replace all the pixels in opposite order.
//    int storage = int(textImg.pixelVector.size());
//    for(unsigned int i = textImg.pixelVector.size() - 1 ; i >= 0; i --)
//    {
//        newImg.pixelVector.at(i) = textImg.pixelVector[storage];
//        storage--;
//    }
//}

bool isEqual(const Image &output, const Image &example)
{
    for(unsigned int i = 0; i < output.pixelVector.size(); i++)
    {
        if(output.pixelVector.at(i).blue != example.pixelVector.at(i).blue) {
            cout << "BLUE VALUES DONT MATCH: " << (int)output.pixelVector.at(i).blue << ", " << (int)example.pixelVector.at(i).blue << endl;
            return false;
        }
        if(output.pixelVector.at(i).green != example.pixelVector.at(i).green) {
            cout << "GREEN VALUES DONT MATCH: " << (int)output.pixelVector.at(i).green << ", " << (int)example.pixelVector.at(i).green << endl;
            return false;
        }
        if(output.pixelVector.at(i).red != example.pixelVector.at(i).red) {
            cout << "RED VALUES DONT MATCH: " << (int)output.pixelVector.at(i).red << ", " << (int)example.pixelVector.at(i).red << endl;
            return false;
        }
    }
    return true;
}

Image combine(const Image &redImg, const Image &blueImg, const Image &greenImg) {
    Image newImageA(blueImg.headerObject);

    for(unsigned int i = 0; i < newImageA.pixelVector.size(); i++)
    {
        newImageA.pixelVector.at(i).blue = blueImg.pixelVector.at(i).blue;

        newImageA.pixelVector.at(i).green = greenImg.pixelVector.at(i).green;

        newImageA.pixelVector.at(i).red = redImg.pixelVector.at(i).red;

    }
    return newImageA;
}

Image separateR(const Image &layerOne) {
    Image newImageR(layerOne.headerObject);

        for(unsigned int i = 0; i < layerOne.pixelVector.size(); i++)
        {
            newImageR.pixelVector.at(i).red = layerOne.pixelVector.at(i).red;
            newImageR.pixelVector.at(i).green = layerOne.pixelVector.at(i).red;
            newImageR.pixelVector.at(i).blue = layerOne.pixelVector.at(i).red;

        }
    return newImageR;
}

Image separateG(const Image &layerOne)
{
    Image newImageG(layerOne.headerObject);

    for(unsigned int i = 0; i < layerOne.pixelVector.size(); i++)
    {
        newImageG.pixelVector.at(i).green = layerOne.pixelVector.at(i).green;
        newImageG.pixelVector.at(i).red = layerOne.pixelVector.at(i).green;
        newImageG.pixelVector.at(i).blue = layerOne.pixelVector.at(i).green;

    }
    return newImageG;
}

Image separateB(const Image &layerOne) {
    Image newImageB(layerOne.headerObject);

    for(unsigned int i = 0; i < layerOne.pixelVector.size(); i++)
    {
        newImageB.pixelVector.at(i).blue = layerOne.pixelVector.at(i).blue;
        newImageB.pixelVector.at(i).green = layerOne.pixelVector.at(i).blue;
        newImageB.pixelVector.at(i).red = layerOne.pixelVector.at(i).blue;

    }
    return newImageB;

}


/* === ONE === */
void TaskOne()
{
    cout << "TASK 1" << endl;
    Image layer1 = loadImage("input/layer1.tga");
    Image pattern1 = loadImage("input/pattern1.tga");
    Image newImg = Multiply(layer1,pattern1);
    outputImage("part1.tga", newImg);

    Image example1 = loadImage("examples/EXAMPLE_part1.tga");
    if(!isEqual(newImg, example1))
    {
        cout << "FAILED\n" << endl;
    }
}

/* === TWO === */
void TaskTwo()
{
    cout << "TASK 2" << endl;
    Image car = loadImage("input/car.tga");
    Image layer2 = loadImage("input/layer2.tga");
    Image newImg = Subtract(layer2, car);
    outputImage("part2.tga", newImg);

    Image example2 = loadImage("examples/EXAMPLE_part2.tga");
    if(!isEqual(newImg, example2))
    {
        cout << "FAILED\n" << endl;
    }
}

/* === THREE === */
void TaskThree()
{
    cout << "TASK 3" << endl;
    Image layer1 = loadImage("input/layer1.tga");
    Image pattern2 = loadImage("input/pattern2.tga");
    Image newImg = Multiply(layer1,pattern2);
    Image text = loadImage("input/text.tga");
    Image newImg2 = Screen(text, newImg);
    outputImage("part3.tga", newImg2);

    Image example3 = loadImage("examples/EXAMPLE_part3.tga");
    if(!isEqual(newImg2, example3))
    {
        cout << "FAILED\n" << endl;
    }

}

/* === FOUR === */
void TaskFour()
{
    cout << "TASK 4" << endl;
    Image layer2 = loadImage("input/layer2.tga");
    Image circles = loadImage("input/circles.tga");
    Image newImg = Multiply(layer2,circles);
    Image pattern2 = loadImage("input/pattern2.tga");
    Image newImg2 = Subtract(pattern2, newImg);
    outputImage("part4.tga", newImg2);

    Image example4 = loadImage("examples/EXAMPLE_part4.tga");
    if(!isEqual(newImg2, example4))
    {
        cout << "FAILED\n" << endl;
    }
}

/* === FIVE === */
void TaskFive()
{
    cout << "TASK 5" << endl;

    Image layer1 = loadImage("input/layer1.tga");
    Image pattern1 = loadImage("input/pattern1.tga");
    Image newImg = Overlay(layer1, pattern1);
    outputImage("part5.tga", newImg);

    Image example5 = loadImage("examples/EXAMPLE_part5.tga");
    if(!isEqual(newImg, example5))
    {
        cout << "FAILED\n" << endl;
    }
}

/* === SIX === */
void TaskSix()
{
    cout << "TASK 6" << endl;
    Image car = loadImage("input/car.tga");
    Image newImg = addChannel(car, "green", 200);
    outputImage("part6.tga", newImg);

    Image example6 = loadImage("examples/EXAMPLE_part6.tga");
    if(!isEqual(newImg, example6))
    {
        cout << "FAILED\n" << endl;
    }
}

/* === SEVEN === */
void TaskSeven()
{
    cout << "TASK 7" << endl;
    Image car = loadImage("input/car.tga");
    Image newImg = scaleChannel(car, "red", 4);
    Image newImg2 = scaleChannel(newImg, "blue", 0);
    outputImage("part7.tga", newImg2);

    Image example7 = loadImage("examples/EXAMPLE_part7.tga");
    if(!isEqual(newImg2, example7))
    {
        cout << "FAILED\n" << endl;
    }
}

/* === EIGHT === */
void TaskEight()
{
    cout << "TASK 8" << endl;
    Image car = loadImage("input/car.tga");
    Image newImgR = separateR(car);
    Image newImgG = separateG(car);
    Image newImgB = separateB(car);

    outputImage("part8_r.tga", newImgR);
    outputImage("part8_g.tga", newImgG);
    outputImage("part8_b.tga", newImgB);

    Image example8_r = loadImage("examples/EXAMPLE_part8_r.tga");
    if(!isEqual(newImgR, example8_r))
    {
        cout << "FAILED\n" << endl;
    }
    Image example8_g = loadImage("examples/EXAMPLE_part8_g.tga");
    if(!isEqual(newImgG, example8_g))
    {
        cout << "FAILED\n" << endl;
    }
    Image example8_b = loadImage("examples/EXAMPLE_part8_b.tga");
    if(!isEqual(newImgB, example8_b))
    {
        cout << "FAILED\n" << endl;
    }
}

/* === NINE === */
void TaskNine()
{

    cout << "TASK 9" << endl;
    Image newImgR = loadImage("input/layer_red.tga");
    Image newImgB = loadImage("input/layer_blue.tga");
    Image newImgG = loadImage("input/layer_green.tga");


    Image newImgFinal = combine(newImgR, newImgB, newImgG);
    outputImage("part9.tga", newImgFinal);
    Image example9 = loadImage("examples/EXAMPLE_part9.tga");
    if(!isEqual(newImgFinal, example9))
    {
        cout << "FAILED\n" << endl;
    }

}

/* === TEN === */
void TaskTen()
{

}

int main()
{
    TaskOne();
    TaskTwo();
    TaskThree();
    TaskFour();
    TaskFive();
    TaskSix();
    TaskSeven();
    TaskEight();
    TaskNine();

}
