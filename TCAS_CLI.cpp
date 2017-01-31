/*
 *  Copyright
 *
 */

#include <iostream>
#include <string.h>
#include <iomanip>

using namespace std;

void printStatusHeader()
{
    cout << "    Hex        Lat         Lon         Alt         HDG         TAS         Vup" << endl;
}

void printStatusHeaderLong()
{
    cout << "    Hex        Lat         Lon         Alt         HDG         TAS         Vup      Status      CRC32       Timeout" << endl;
}

void printStatusDisp(long long ac_hex, double lat, double lon, double altitude, double hdg, double TAS, double Vup)
{
    cout << fixed;
    cout.precision(4);
    cout << "   " << hex << setw(8) << ac_hex << "    ";
    cout << setw(7) << lat << "      " << setw(8) << lon << "  ";
    cout.precision(0);
    cout << setw(5) << altitude << "        ";
    cout.precision(1);
    cout << setw(4) << hdg << "       " << setw(4) << TAS << "       ";
    cout.precision(2);
    cout << setw(5) << Vup;
}

void printStatusDisp(long long ac_hex, 
                    double lat, double lon, double altitude, 
                    double hdg, double TAS, double Vup,
                    std::string status, bool CRCpass, int timeout)
{
    printStatusDisp(ac_hex, lat, lon, altitude, hdg, TAS, Vup);
    cout << "    ";
    if (status == "CLEAR")
    {
        cout << "CLEAR       ";
    }
    else if (status == "ADVISORY")
    {
        cout << "ADVISORY    ";
    }
    else if (status == "RESOLUTION")
    {
        cout << "RESOLUTION  ";
    }
    else if (status == "RETURNING")
    {
        cout << "RETURNING   ";
    }
    else
    {
        cout << "???         ";
    }

    if (CRCpass)
    {
        cout << "OK          ";
    }
    else
    {
        cout << "FAIL!       ";
    }

    cout << setw(1) << timeout;
}
