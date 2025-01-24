/* mbed ID12 RFID Library
 * Copyright (c) 2007-2010, sford, http://mbed.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#ifndef MBED_ID12RFID_H
#define MBED_ID12RFID_H
 
#include "mbed.h"

/** An interface for the ID-12 RFID reader device
 *
 * @code
 * // Print RFID tag numbers
 * 
 * #include "mbed.h"
 * #include "ID12RFID.h"
 * 
 * ID12RFID rfid(p10); // serial rx
 * 
 * int main() {
 *     while(1) {
 *         if(rfid.readable()) {
 *             printf("RFID Tag number : %d\n", rfid.read());             
 *         }
 *     }
 * }
 * @endcode
 */
class ID12RFID {

public:
    /** Create an ID12 RFID interface, connected to the specified Serial rx port
     *
     * @param rx Recieve pin 
     */
    ID12RFID(PinName rx);

    /** Non blocking function to determine if an ID has been received
     *
     * @return Non zero value when the device is readable
     */
    int readable();    
    
    /** A blocking function that will return a tag ID when available
     *
     * @return The ID tag value
     */
    int read();

private:
    UnbufferedSerial _rfid;
    
};

#endif