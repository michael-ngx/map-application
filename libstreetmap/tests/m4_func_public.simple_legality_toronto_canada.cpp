#include <random>
#include <iostream>
#include <UnitTest++/UnitTest++.h>

#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m3.h"
#include "m4.h"

#include "unit_test_util.h"
#include "courier_verify.h"

using ece297test::relative_error;
using ece297test::courier_path_is_legal;


SUITE(simple_legality_toronto_canada_public) {
    TEST(simple_legality_toronto_canada) {
        std::vector<DeliveryInf> deliveries;
        std::vector<IntersectionIdx> depots;
        std::vector<CourierSubPath> result_path;
        float turn_penalty;

        deliveries = {DeliveryInf(23285, 30394), DeliveryInf(65052, 98292), DeliveryInf(69434, 112840), DeliveryInf(165581, 51879), DeliveryInf(76559, 147917)};
        depots = {82393, 91986, 83785};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(33059, 39404)};
        depots = {8};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(30720, 67693), DeliveryInf(36317, 25933), DeliveryInf(129351, 151543)};
        depots = {14501, 1618, 181726};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(41283, 54262), DeliveryInf(24164, 92899), DeliveryInf(66787, 70120), DeliveryInf(150554, 155285), DeliveryInf(88907, 2754)};
        depots = {81319, 142913, 52108};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(32645, 70504), DeliveryInf(45370, 30769)};
        depots = {86936, 109003};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(73536, 17212)};
        depots = {153739, 112665};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(119925, 5790)};
        depots = {169607, 161209};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(154741, 102215)};
        depots = {5822};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(31970, 120356), DeliveryInf(35737, 4553)};
        depots = {94675};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(124331, 156932), DeliveryInf(25964, 156932), DeliveryInf(67812, 156932), DeliveryInf(153404, 97799), DeliveryInf(68424, 91419), DeliveryInf(94361, 91419), DeliveryInf(180613, 151301), DeliveryInf(127593, 64272)};
        depots = {14187, 6615, 128524};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(42566, 168058)};
        depots = {35927, 36402};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(51536, 48633), DeliveryInf(116224, 105642), DeliveryInf(37296, 57573), DeliveryInf(36175, 76961), DeliveryInf(36175, 105160), DeliveryInf(58813, 34544), DeliveryInf(36175, 154104), DeliveryInf(116224, 154214)};
        depots = {34099, 58669, 99147};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(61863, 59982), DeliveryInf(94319, 91605), DeliveryInf(162621, 37423), DeliveryInf(94319, 59982), DeliveryInf(163417, 157781), DeliveryInf(61863, 157781), DeliveryInf(94319, 59982), DeliveryInf(157015, 157781), DeliveryInf(94319, 171145)};
        depots = {120, 180420, 183551};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(64028, 16077), DeliveryInf(76981, 94877), DeliveryInf(86521, 44714), DeliveryInf(86521, 66147), DeliveryInf(2820, 27792), DeliveryInf(86521, 40655), DeliveryInf(2820, 80161), DeliveryInf(67514, 142237)};
        depots = {91400, 188386, 180980};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(87798, 123069), DeliveryInf(46320, 23656), DeliveryInf(46284, 187909), DeliveryInf(26682, 187315), DeliveryInf(119471, 23656), DeliveryInf(186303, 23656), DeliveryInf(121158, 187315), DeliveryInf(157992, 140734)};
        depots = {19232, 147604, 28202};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(191433, 106080), DeliveryInf(27861, 74592)};
        depots = {38703, 134104};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(64038, 139182), DeliveryInf(158177, 114314)};
        depots = {59484};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(118395, 34964), DeliveryInf(56296, 36226), DeliveryInf(125453, 184998), DeliveryInf(109945, 170985), DeliveryInf(9479, 152574)};
        depots = {171121, 145232, 43113};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(127693, 57367), DeliveryInf(114452, 28017), DeliveryInf(12655, 175901), DeliveryInf(3657, 59453), DeliveryInf(12655, 113771), DeliveryInf(160388, 110274), DeliveryInf(12655, 33178), DeliveryInf(127693, 109020)};
        depots = {98401, 74879, 124043};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(130185, 177203), DeliveryInf(29632, 50075), DeliveryInf(67853, 171758)};
        depots = {29959, 63731, 77183};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(130308, 168964), DeliveryInf(117529, 54933), DeliveryInf(65839, 168964), DeliveryInf(117349, 54933), DeliveryInf(158825, 54933), DeliveryInf(183286, 116918), DeliveryInf(99424, 100534), DeliveryInf(60610, 96438)};
        depots = {42980, 124042, 76398};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(153059, 158357), DeliveryInf(88301, 95506), DeliveryInf(125001, 124815), DeliveryInf(56466, 91817), DeliveryInf(92214, 19670)};
        depots = {65431, 110489, 18689};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(168073, 13481), DeliveryInf(112155, 170357), DeliveryInf(34092, 146980), DeliveryInf(32781, 191405), DeliveryInf(125387, 68330)};
        depots = {68441, 178657, 7075};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(174871, 19498), DeliveryInf(159027, 156001), DeliveryInf(45995, 80919), DeliveryInf(95061, 9005), DeliveryInf(18407, 146924)};
        depots = {184489, 68192, 49755};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(190819, 44152), DeliveryInf(144100, 47529), DeliveryInf(67125, 44339)};
        depots = {74205, 64305, 178278};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(74564, 95592), DeliveryInf(74564, 170932), DeliveryInf(40700, 80305), DeliveryInf(52742, 80305), DeliveryInf(191237, 151477), DeliveryInf(74564, 3167), DeliveryInf(74564, 3167), DeliveryInf(6621, 80305), DeliveryInf(40700, 3167)};
        depots = {135046, 186144, 26974};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(80423, 70861), DeliveryInf(83843, 70861), DeliveryInf(55387, 116282), DeliveryInf(83140, 70861), DeliveryInf(69606, 90664), DeliveryInf(69606, 177838), DeliveryInf(69606, 153336), DeliveryInf(80423, 177838), DeliveryInf(69606, 177838)};
        depots = {133550, 77131, 171788};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(170618, 160175), DeliveryInf(158972, 191403)};
        depots = {93042, 122131};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(181414, 115510), DeliveryInf(165644, 155320)};
        depots = {146431};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

        deliveries = {DeliveryInf(181936, 73749)};
        depots = {152254};
        turn_penalty = 30.000000000;
        result_path = travelingCourier(deliveries, depots, turn_penalty);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));

    } //simple_legality_toronto_canada

} //simple_legality_toronto_canada_public

