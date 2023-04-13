/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include <string>

#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "m4.h"
#include "globals.h"

//Program exit codes
constexpr int SUCCESS_EXIT_CODE = 0;        //Everyting went OK
constexpr int ERROR_EXIT_CODE = 1;          //An error occured
constexpr int BAD_ARGUMENTS_EXIT_CODE = 2;  //Invalid command-line usage

//The default map to load if none is specified
std::string default_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
// std::string default_map_path = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
// The start routine of your program (main) when you are running your standalone
// mapper program. This main routine is *never called* when you are running 
// ece297exercise (the unit tests) -- those tests have their own main routine
// and directly call your functions in /libstreetmap/src/ to test them.
// Don't write any code in this file that you want run by ece297exerise -- it 
// will not be called!
int main(int argc, char** argv) {

    std::string map_path;
    if(argc == 1) {
        //Use a default map
        map_path = default_map_path;
    } else if (argc == 2) {
        //Get the map from the command line
        map_path = argv[1];
    } else {
        //Invalid arguments
        std::cerr << "Usage: " << argv[0] << " [map_file_path]\n";
        std::cerr << "  If no map_file_path is provided a default map is loaded.\n";
        return BAD_ARGUMENTS_EXIT_CODE;
    }

    //Load the map and related data structures
    bool load_success = loadMap(map_path);
    if(!load_success) {
        std::cerr << "Failed to load map '" << map_path << "'\n";
        return ERROR_EXIT_CODE;
    }

    std::cout << "Successfully loaded map '" << map_path << "'\n";

    // Update the CURRENT_CITY based on first input
    CURRENT_MAP_PATH = map_path;

    //You can now do something with the map data
    std::vector<DeliveryInf> deliveries;
    std::vector<IntersectionIdx> depots;
    float turn_penalty;
    std::vector<CourierSubPath> result_path;

    // deliveries = {DeliveryInf(175352, 112936), DeliveryInf(50632, 53620), DeliveryInf(152619, 166072), DeliveryInf(28305, 111405), DeliveryInf(10948, 142450), DeliveryInf(48872, 17183), DeliveryInf(27204, 68318), DeliveryInf(61599, 115394), DeliveryInf(75, 15079), DeliveryInf(56272, 183872), DeliveryInf(109883, 33289), DeliveryInf(89979, 174143), DeliveryInf(29415, 147902), DeliveryInf(43091, 33785), DeliveryInf(144547, 25733), DeliveryInf(183579, 63568), DeliveryInf(155334, 45089), DeliveryInf(110877, 33557), DeliveryInf(109040, 38673), DeliveryInf(94366, 3755), DeliveryInf(102862, 22127), DeliveryInf(103831, 165443), DeliveryInf(862, 113022), DeliveryInf(149297, 22564), DeliveryInf(137675, 171695), DeliveryInf(180761, 190746), DeliveryInf(11310, 49229), DeliveryInf(150543, 125445), DeliveryInf(137742, 125639), DeliveryInf(15496, 62346), DeliveryInf(60234, 62528), DeliveryInf(69484, 25179), DeliveryInf(82059, 44660), DeliveryInf(70414, 193857), DeliveryInf(82796, 29026), DeliveryInf(114259, 153591), DeliveryInf(57991, 163444), DeliveryInf(11451, 30358), DeliveryInf(86831, 188073), DeliveryInf(142471, 85973), DeliveryInf(157442, 143138), DeliveryInf(5018, 178697), DeliveryInf(168310, 52430), DeliveryInf(102736, 49720), DeliveryInf(7660, 1858), DeliveryInf(52370, 28866), DeliveryInf(67314, 29767), DeliveryInf(106605, 63275), DeliveryInf(178733, 147886), DeliveryInf(125118, 88823), DeliveryInf(74969, 42680), DeliveryInf(137986, 48715), DeliveryInf(117463, 98315), DeliveryInf(41904, 58317), DeliveryInf(66859, 141477), DeliveryInf(1149, 11085), DeliveryInf(73891, 128355), DeliveryInf(43440, 63788), DeliveryInf(183924, 172763), DeliveryInf(108991, 141958), DeliveryInf(173255, 110264), DeliveryInf(131921, 99840), DeliveryInf(57666, 17742), DeliveryInf(57645, 112461), DeliveryInf(139729, 184597), DeliveryInf(33069, 112741), DeliveryInf(107637, 181070), DeliveryInf(126327, 119523), DeliveryInf(38669, 8191), DeliveryInf(141612, 140766), DeliveryInf(52811, 139312), DeliveryInf(10582, 80074), DeliveryInf(34054, 86734), DeliveryInf(127408, 144381), DeliveryInf(50756, 3238), DeliveryInf(13977, 24763), DeliveryInf(60406, 173270), DeliveryInf(79687, 141382), DeliveryInf(124808, 73000), DeliveryInf(30947, 421), DeliveryInf(18365, 119132), DeliveryInf(154534, 98854), DeliveryInf(28664, 145221), DeliveryInf(82029, 41122), DeliveryInf(63963, 119323), DeliveryInf(76166, 130374), DeliveryInf(170689, 175833), DeliveryInf(24221, 93215), DeliveryInf(57974, 181578), DeliveryInf(107762, 144492), DeliveryInf(120906, 48961), DeliveryInf(37121, 136447), DeliveryInf(97515, 69901), DeliveryInf(164881, 73511), DeliveryInf(141492, 133240), DeliveryInf(175444, 158466), DeliveryInf(51785, 120351), DeliveryInf(90767, 142346), DeliveryInf(12802, 5073), DeliveryInf(103715, 19240), DeliveryInf(6391, 25593), DeliveryInf(66680, 96596), DeliveryInf(52856, 142555), DeliveryInf(86169, 86143), DeliveryInf(152493, 180385), DeliveryInf(38265, 53233), DeliveryInf(126769, 162609), DeliveryInf(175263, 115271), DeliveryInf(110997, 17462), DeliveryInf(126901, 164183), DeliveryInf(17970, 157473), DeliveryInf(15694, 49657), DeliveryInf(9872, 133063), DeliveryInf(172943, 140177), DeliveryInf(10324, 53698), DeliveryInf(127111, 50064), DeliveryInf(150574, 68846), DeliveryInf(69157, 29360), DeliveryInf(30626, 34615), DeliveryInf(30673, 46368), DeliveryInf(129668, 145259), DeliveryInf(28547, 66580), DeliveryInf(65325, 127756), DeliveryInf(190965, 61230), DeliveryInf(45096, 148126), DeliveryInf(108429, 64796), DeliveryInf(172367, 147496), DeliveryInf(1509, 181774), DeliveryInf(131278, 54798), DeliveryInf(164265, 6581), DeliveryInf(181881, 192266), DeliveryInf(171824, 139149), DeliveryInf(28308, 64425), DeliveryInf(22149, 87088), DeliveryInf(18925, 57788), DeliveryInf(89920, 167807), DeliveryInf(53762, 160393), DeliveryInf(172236, 135722), DeliveryInf(67632, 9182), DeliveryInf(184858, 129409), DeliveryInf(160179, 156441), DeliveryInf(170216, 17759), DeliveryInf(104309, 103425), DeliveryInf(29740, 23176), DeliveryInf(93388, 90604), DeliveryInf(169617, 67288), DeliveryInf(115193, 64307), DeliveryInf(177772, 60619), DeliveryInf(156500, 28376), DeliveryInf(57635, 103349), DeliveryInf(162120, 158243), DeliveryInf(142050, 42946), DeliveryInf(146507, 20424), DeliveryInf(161137, 186200), DeliveryInf(46543, 93982), DeliveryInf(107984, 112900), DeliveryInf(50247, 95120), DeliveryInf(13690, 90222), DeliveryInf(146027, 145484), DeliveryInf(86599, 157213), DeliveryInf(119248, 54990), DeliveryInf(132460, 144750), DeliveryInf(130538, 73275), DeliveryInf(76486, 92721), DeliveryInf(173666, 24941), DeliveryInf(37027, 68264), DeliveryInf(10495, 85181), DeliveryInf(165198, 128919), DeliveryInf(102875, 94411), DeliveryInf(83180, 17536), DeliveryInf(75370, 73562), DeliveryInf(40851, 48626), DeliveryInf(45361, 126794), DeliveryInf(13443, 156001), DeliveryInf(164547, 31517)};
    // depots = {30, 115709, 40719, 46903, 150790, 63728, 117576, 152806, 163358, 39770, 121853, 179374, 14697, 129655, 27882, 113135, 26522, 187706, 16228, 148961};
    // turn_penalty = 30.000000000;
    // result_path = travelingCourier(deliveries, depots, turn_penalty);
    // std::cout << result_path.size() << std::endl;

    deliveries = {DeliveryInf(33059, 39404)};
    depots = {8};
    turn_penalty = 30.000000000;
    result_path = travelingCourier(deliveries, depots, turn_penalty);



    // drawMap();

    //Clean-up the map data and related data structures
    std::cout << "Closing map\n";
    closeMap(); 

    return SUCCESS_EXIT_CODE;
}
