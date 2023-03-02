
#include "m1.h"
#include "unit_test_util.h"

#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

#include <UnitTest++/UnitTest++.h>

#include <random>
#include <algorithm>
#include <set>

using ece297test::relative_error;
using ece297test::sorted;

SUITE(street_queries_public_toronto_canada) {

    struct BaseMapFixture {
        BaseMapFixture() {
            //Load the map
            try {
                loadMap("/cad2/ece297s/public/maps/toronto_canada.streets.bin");
            } catch (...) {
                std::cout << "!!!! BaseMapFixture test setup: loadMap threw an exceptinon !!!!" << std::endl;
                throw; // re-throw exceptinon
            }
        }
    
        ~BaseMapFixture() {
            //Clean-up
            try {
                closeMap();
            } catch (const std::exception& e) {
                std::cout << "!!!! BaseMapFixture test teardown: closeMap threw an exceptinon. what(): " << e.what() << " !!!!" << std::endl;
                std::terminate(); // we're in a destructor
            } catch (...) {
                std::cout << "!!!! BaseMapFixture test teardown: closeMap threw an exceptinon !!!!" << std::endl;
                std::terminate(); // we're in a destructor
            }
        }
    };


    struct MapFixture : BaseMapFixture {};

    TEST_FIXTURE(MapFixture, all_street_intersections) {
        std::vector<IntersectionIdx> expected;

        expected = {2, 3, 10, 745, 746, 749, 750, 751, 752, 755, 756, 759, 760, 763, 764, 772, 773, 774, 776, 777, 781, 1917, 1934, 7470, 10465, 10472, 10473, 10597, 10760, 10761, 10768, 10769, 10807, 10808, 14060, 14067, 14072, 15743, 15744, 15745, 15786, 15787, 15904, 15905, 15906, 17904, 17905, 20440, 21452, 25925, 25948, 25949, 31104, 31105, 31106, 31117, 31118, 31119, 31437, 31438, 31439, 31448, 31449, 41767, 41768, 62961, 62962, 63317, 63318, 63331, 80044, 80058, 80059, 80060, 80073, 81003, 81004, 81508, 81509, 81514, 81517, 81518, 81525, 98166, 98415, 98424, 104527, 104528, 121595};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(2)));

        expected = {12182, 24011, 24034};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(5219)));

        expected = {18866, 18867, 117097, 117098};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(4044)));

        expected = {20392, 20393, 20394, 20397};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(4392)));

        expected = {21808, 170437, 170439};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(22301)));

        expected = {26977, 26978, 26979};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(5774)));

        expected = {28316, 28317, 28318};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(6081)));

        expected = {29890, 29891, 35415, 35416, 35465, 39199, 39201, 39218, 86431, 86432, 86433, 86434, 86435, 86436, 86437, 86438, 86439, 86457, 86458, 86459, 86460, 86461, 86462, 86463, 86464, 86465, 86466, 86467, 86480, 86481, 86482, 86483, 103471, 103472, 103473, 103474, 103475, 113313, 119360, 119362, 119365, 119366, 120226, 131661, 136847, 136848, 142221, 142222, 142223, 142224};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(6296)));

        expected = {31941, 106425, 106426, 106427, 106428, 106429, 106430};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20023)));

        expected = {33680, 77421, 77437};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15949)));

        expected = {47154, 64964};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12881)));

        expected = {52142, 53958};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(9704)));

        expected = {53287, 62356, 62357, 62358, 62363};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12113)));

        expected = {58851, 58873};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(11185)));

        expected = {63016, 63030};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12334)));

        expected = {66025, 66026, 66036};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12992)));

        expected = {71160, 71161, 71163};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(14258)));

        expected = {75108, 75152};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15275)));

        expected = {75576, 75599};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15418)));

        expected = {77033, 77034, 77040, 77042, 77045};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15810)));

        expected = {78715, 78716, 78718, 78719, 78720, 78723, 78731, 78732, 78738, 78740, 103190};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(16309)));

        expected = {80671, 80673, 80674, 80675, 80676, 80677, 80678, 80679, 80683, 80685, 80688, 80693, 80780};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(16901)));

        expected = {87865, 87866};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(17559)));

        expected = {88804, 88805, 88806, 88807, 88808, 88809, 88810};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(17852)));

        expected = {89280, 89694};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(18200)));

        expected = {99516, 115090, 115092};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20459)));

        expected = {104834, 104835, 192022};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(19966)));

        expected = {108753, 108754};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20174)));

        expected = {112225, 112235};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20367)));

        expected = {134336, 142366};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(21707)));

    } //all_street_intersections

    TEST_FIXTURE(MapFixture, intersection_ids_from_street_ids) {
        std::vector<IntersectionIdx> expected;

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(4281, 6514)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(6343, 4530)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(7364, 2533)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(7712, 13851)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(8982, 17636)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(10247, 17317)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(11275, 3834)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(11600, 9105)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(12961, 11231)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(13442, 18367)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(15617, 18451)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(16683, 506)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(19062, 17273)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(19619, 407)));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(19820, 22010)));

        expected = {2, 3, 10, 746, 750, 759, 773, 776, 1917, 1934, 7470, 10465, 14060, 14067, 14072, 20440, 21452, 31119, 81518};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(2, 0)));

        expected = {7675, 7676};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(1527, 0)));

        expected = {12297};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(19242, 2594)));

        expected = {33680, 77437};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(15949, 0)));

        expected = {36788};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(7165, 16721)));

        expected = {46676};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(8315, 8172)));

        expected = {78738, 78740};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(16309, 16320)));

        expected = {79395, 79403};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(16550, 16543)));

        expected = {80674, 80675, 80677, 80678, 80693, 80780};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(16901, 0)));

        expected = {87881};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(4867, 17568)));

        expected = {88805};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(17852, 17888)));

        expected = {89484, 89683};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(18077, 18156)));

        expected = {108753};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(20174, 20176)));

        expected = {115090};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(20459, 19373)));

        expected = {127129, 127130, 127131, 127132};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(20945, 0)));

    } //intersection_ids_from_street_ids

    TEST_FIXTURE(MapFixture, street_ids_from_partial_street_name) {
        std::vector<StreetIdx> expected;

        expected = {1, 2, 3, 4, 10, 31, 42, 62, 63, 120, 121, 125, 126, 127, 128, 131, 132, 133, 135, 144, 145, 146, 147, 148, 149, 150, 220, 221, 500, 560, 562, 612, 613, 890, 891, 892, 957, 1346, 1352, 1353, 1575, 1596, 1726, 1727, 1728, 1729, 1730, 1946, 1947, 2020, 2033, 2140, 2287, 2317, 2481, 2546, 2861, 2878, 3157, 3275, 3560, 3659, 3718, 4696, 4718, 4882, 5238, 5659, 5795, 5899, 5900, 6242, 6367, 6763, 7528, 8190, 8203, 8312, 9478, 10059, 10264, 10636, 10643, 11034, 11091, 11508, 11593, 11619, 11892, 11916, 12107, 12382, 12424, 12911, 13283, 13404, 13519, 14045, 14855, 15220, 15550, 16219, 16231, 16486, 17092, 18053, 18536, 18616, 19144, 19172, 19699, 20850, 21123, 21262, 21582, 22475};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("High")));

        expected = {23, 5341, 9951};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Old Yonge S")));

        expected = {103, 22571};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Commiss")));

        expected = {484, 841, 894, 1143, 1660, 2746, 3386, 3572, 4383, 5003, 5989, 6300, 6858, 7520, 8627, 9876, 14016, 14486, 14515, 15098, 15308, 16091, 16513, 18064, 18305, 18586, 18735, 19251, 22282, 22310};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Ham")));

        expected = {652, 16550};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Plains")));

        expected = {694, 1405, 1759, 1839, 1993, 2321, 2483, 2847, 2994, 3064, 3675, 4015, 4110, 4290, 4460, 4833, 5132, 5507, 6035, 6062, 6063, 6370, 7162, 7286, 7864, 8140, 8264, 8403, 8585, 8652, 8811, 8887, 8897, 9348, 9425, 10021, 10066, 10147, 10177, 10360, 10518, 10541, 10621, 10725, 11009, 11198, 11216, 11245, 11329, 11577, 11742, 12101, 12143, 12334, 12335, 12551, 12625, 12724, 12730, 12745, 12880, 13820, 14293, 14531, 14955, 15304, 15658, 16233, 16466, 16533, 16835, 17166, 17375, 17613, 17710, 17876, 18411, 18413, 18778, 19321, 19926, 20093, 20346, 21097, 21806, 21872};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("San")));

        expected = {1527};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Chapman Road")));

        expected = {1877, 2000, 2440, 3286, 3427, 3504, 4139, 4160, 4307, 4765, 4877, 4915, 5660, 5678, 5827, 5993, 6322, 6492, 7192, 7682, 7891, 8091, 8362, 8466, 8483, 8495, 8696, 9548, 9618, 9998, 10139, 10336, 10434, 10603, 10728, 11312, 11447, 12104, 12213, 12329, 12402, 12913, 13402, 13552, 14428, 14525, 14561, 14886, 14977, 15947, 16014, 16044, 16090, 16103, 16107, 16157, 16245, 16260, 16484, 16525, 16549, 16669, 17042, 17141, 17233, 17554, 17652, 17684, 18312, 18439, 18440, 18574, 19721, 19764, 20925, 21046, 21725, 21856, 22474};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Pin")));

        expected = {2533, 4651, 16587};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Elvina ")));

        expected = {3626, 8982};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Allard Avenue")));

        expected = {4044};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Dallington Drive")));

        expected = {4530};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Thursfiel")));

        expected = {4933};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Omni Driv")));

        expected = {5219};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Dover Drive")));

        expected = {6296};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Winston Park Drive")));

        expected = {6358};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Rossburn Dr")));

        expected = {6395, 12314};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("King Henrys Boulevard")));

        expected = {7210};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Roseland Gate")));

        expected = {8746};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("McRae Road")));

        expected = {9973, 14342, 16901, 21898};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Beech Street")));

        expected = {12113};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Trailridge Crescent")));

        expected = {12961};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Elana")));

        expected = {12992};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Conamore Cre")));

        expected = {15184};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Age")));

        expected = {15275};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Clipperton Dri")));

        expected = {15949};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Mimosa Row")));

        expected = {17273};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Fishing Crescent")));

        expected = {18077, 19333};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Royal Coun")));

        expected = {19444, 22010};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("John Birchall Road")));

        expected = {19619};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Cedarsprings Way")));

    } //street_ids_from_partial_street_name

} //street_queries_public_toronto_canada

