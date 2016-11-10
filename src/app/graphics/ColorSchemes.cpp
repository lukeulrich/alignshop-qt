/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QPair>
#include <QtGui/QColor>

#include "ColorSchemes.h"

#include "../graphics/LinearColorScheme.h"
#include "../graphics/SymbolColorScheme.h"
#include "../graphics/TextColorStyle.h"

namespace ColorSchemes
{
    SymbolColorScheme initializeClustalAminoColorScheme()
    {
        QColor red(.9 * 255, .2 * 255, .1 * 255);
        QColor blue(.1 * 255, .5 * 255, .9 * 255);
        QColor green(.1 * 255, .8 * 255, .1 * 255);
        QColor cyan(.1 * 255, .7 * 255, .7 * 255);
        QColor pink(.9 * 255, .5 * 255, .5 * 255);
        QColor magenta(.8 * 255, .3 * 255, .8 * 255);
        QColor yellow(.8 * 255, .8 * 255, 0);
        QColor orange(.9 * 255, .6 * 255, .3 * 255);

        SymbolColorScheme clustalColorScheme;
        clustalColorScheme.setTextColorStyle('G', TextColorStyle(Qt::black, orange));
        clustalColorScheme.setTextColorStyle('P', TextColorStyle(Qt::black, yellow));
        clustalColorScheme.setSymbolsTextColorStyle('T', "tST%#", TextColorStyle(Qt::black, green));
        clustalColorScheme.setSymbolsTextColorStyle('S', "tST#", TextColorStyle(Qt::black, green));
        clustalColorScheme.setSymbolsTextColorStyle('N', "nND", TextColorStyle(Qt::black, green));
        clustalColorScheme.setSymbolsTextColorStyle('Q', "qQE+KR", TextColorStyle(Qt::black, green));

        clustalColorScheme.setSymbolsTextColorStyle('W', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('L', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('V', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('I', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('M', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('A', "%#ACFHILMVWYPpTSsG", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('F', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('C', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, blue));
        clustalColorScheme.setSymbolsTextColorStyle('C', "C", TextColorStyle(Qt::black, pink));
        clustalColorScheme.setSymbolsTextColorStyle('H', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, cyan));
        clustalColorScheme.setSymbolsTextColorStyle('Y', "%#ACFHILMVWYPp", TextColorStyle(Qt::black, cyan));
        clustalColorScheme.setSymbolsTextColorStyle('E', "-DEqQ", TextColorStyle(Qt::black, magenta));
        clustalColorScheme.setSymbolsTextColorStyle('D', "-DEnN", TextColorStyle(Qt::black, magenta));
        clustalColorScheme.setSymbolsTextColorStyle('K', "+KRQ", TextColorStyle(Qt::black, red));
        clustalColorScheme.setSymbolsTextColorStyle('R', "+KRQ", TextColorStyle(Qt::black, red));

        return clustalColorScheme;
    }
    const SymbolColorScheme kClustalAminoScheme(initializeClustalAminoColorScheme());

    CharColorScheme initializeZappoAminoColorScheme()
    {
        CharColorScheme zappo;

        QColor peach(255, 175, 175);
        zappo.setTextColorStyle('I', TextColorStyle(Qt::black, peach));
        zappo.setTextColorStyle('L', TextColorStyle(Qt::black, peach));
        zappo.setTextColorStyle('V', TextColorStyle(Qt::black, peach));
        zappo.setTextColorStyle('A', TextColorStyle(Qt::black, peach));
        zappo.setTextColorStyle('M', TextColorStyle(Qt::black, peach));

        QColor yellowOrange(255, 200, 0);
        zappo.setTextColorStyle('F', TextColorStyle(Qt::black, yellowOrange));
        zappo.setTextColorStyle('W', TextColorStyle(Qt::black, yellowOrange));
        zappo.setTextColorStyle('Y', TextColorStyle(Qt::black, yellowOrange));

        QColor bluePurple(100, 100, 255);
        zappo.setTextColorStyle('K', TextColorStyle(Qt::black, bluePurple));
        zappo.setTextColorStyle('R', TextColorStyle(Qt::black, bluePurple));
        zappo.setTextColorStyle('H', TextColorStyle(Qt::black, bluePurple));

        zappo.setTextColorStyle('D', TextColorStyle(Qt::black, Qt::red));
        zappo.setTextColorStyle('E', TextColorStyle(Qt::black, Qt::red));

        zappo.setTextColorStyle('S', TextColorStyle(Qt::black, Qt::green));
        zappo.setTextColorStyle('T', TextColorStyle(Qt::black, Qt::green));
        zappo.setTextColorStyle('N', TextColorStyle(Qt::black, Qt::green));
        zappo.setTextColorStyle('Q', TextColorStyle(Qt::black, Qt::green));

        zappo.setTextColorStyle('P', TextColorStyle(Qt::black, Qt::magenta));
        zappo.setTextColorStyle('G', TextColorStyle(Qt::black, Qt::magenta));

        zappo.setTextColorStyle('C', TextColorStyle(Qt::black, Qt::yellow));

        return zappo;
    }
    const CharColorScheme kZappoAminoScheme(initializeZappoAminoColorScheme());

    CharColorScheme initializeTaylorAminoColorScheme()
    {
        CharColorScheme taylor;

        taylor.setTextColorStyle('A', TextColorStyle(Qt::black, QColor(204, 255, 0)));
        taylor.setTextColorStyle('V', TextColorStyle(Qt::black, QColor(153, 255, 0)));
        taylor.setTextColorStyle('I', TextColorStyle(Qt::black, QColor(102, 255, 0)));
        taylor.setTextColorStyle('L', TextColorStyle(Qt::black, QColor(51, 255, 0)));
        taylor.setTextColorStyle('M', TextColorStyle(Qt::black, Qt::green));
        taylor.setTextColorStyle('F', TextColorStyle(Qt::black, QColor(0, 255, 102)));
        taylor.setTextColorStyle('Y', TextColorStyle(Qt::black, QColor(0, 255, 204)));
        taylor.setTextColorStyle('W', TextColorStyle(Qt::black, QColor(0, 204, 255)));
        taylor.setTextColorStyle('H', TextColorStyle(Qt::black, QColor(0, 102, 255)));
        taylor.setTextColorStyle('R', TextColorStyle(Qt::black, Qt::blue));
        taylor.setTextColorStyle('K', TextColorStyle(Qt::black, QColor(102, 0, 255)));
        taylor.setTextColorStyle('N', TextColorStyle(Qt::black, QColor(204, 0, 255)));
        taylor.setTextColorStyle('Q', TextColorStyle(Qt::black, QColor(255, 0, 204)));
        taylor.setTextColorStyle('E', TextColorStyle(Qt::black, QColor(255, 0, 102)));
        taylor.setTextColorStyle('D', TextColorStyle(Qt::black, Qt::red));
        taylor.setTextColorStyle('S', TextColorStyle(Qt::black, QColor(255, 51, 0)));
        taylor.setTextColorStyle('T', TextColorStyle(Qt::black, QColor(255, 102, 0)));
        taylor.setTextColorStyle('G', TextColorStyle(Qt::black, QColor(255, 153, 0)));
        taylor.setTextColorStyle('P', TextColorStyle(Qt::black, QColor(255, 204, 0)));
        taylor.setTextColorStyle('C', TextColorStyle(Qt::black, Qt::yellow));

        return taylor;
    }
    const CharColorScheme kTaylorAminoScheme(initializeTaylorAminoColorScheme());

    CharColorScheme initializeHydrophobicityAminoColorScheme()
    {
        CharColorScheme hydro;

        hydro.setTextColorStyle('I', TextColorStyle(Qt::black, Qt::red));
        hydro.setTextColorStyle('V', TextColorStyle(Qt::black, QColor(246, 0, 9)));
        hydro.setTextColorStyle('L', TextColorStyle(Qt::black, QColor(234, 0, 21)));
        hydro.setTextColorStyle('F', TextColorStyle(Qt::black, QColor(203, 0, 52)));
        hydro.setTextColorStyle('C', TextColorStyle(Qt::black, QColor(194, 0, 61)));
        hydro.setTextColorStyle('M', TextColorStyle(Qt::black, QColor(176, 0, 79)));
        hydro.setTextColorStyle('A', TextColorStyle(Qt::black, QColor(173, 0, 82)));
        hydro.setTextColorStyle('G', TextColorStyle(Qt::black, QColor(106, 0, 149)));
        hydro.setTextColorStyle('X', TextColorStyle(Qt::black, QColor(104, 0, 151)));
        hydro.setTextColorStyle('T', TextColorStyle(Qt::black, QColor(97, 0, 158)));
        hydro.setTextColorStyle('S', TextColorStyle(Qt::black, QColor(94, 0, 161)));
        hydro.setTextColorStyle('W', TextColorStyle(Qt::black, QColor(91, 0, 164)));
        hydro.setTextColorStyle('Y', TextColorStyle(Qt::black, QColor(79, 0, 176)));
        hydro.setTextColorStyle('P', TextColorStyle(Qt::black, QColor(70, 0, 185)));
        hydro.setTextColorStyle('H', TextColorStyle(Qt::black, QColor(21, 0, 234)));
        hydro.setTextColorStyle('E', TextColorStyle(Qt::black, QColor(12, 0, 234)));
        hydro.setTextColorStyle('Z', TextColorStyle(Qt::black, QColor(12, 0, 234)));
        hydro.setTextColorStyle('Q', TextColorStyle(Qt::black, QColor(12, 0, 234)));
        hydro.setTextColorStyle('D', TextColorStyle(Qt::black, QColor(12, 0, 234)));
        hydro.setTextColorStyle('B', TextColorStyle(Qt::black, QColor(12, 0, 234)));
        hydro.setTextColorStyle('N', TextColorStyle(Qt::black, QColor(12, 0, 234)));
        hydro.setTextColorStyle('K', TextColorStyle(Qt::black, Qt::blue));
        hydro.setTextColorStyle('R', TextColorStyle(Qt::black, Qt::blue));

        return hydro;
    }
    const CharColorScheme kHydrophobocityScheme(initializeHydrophobicityAminoColorScheme());

    CharColorScheme initializeHelixPropensityAminoColorScheme()
    {
        CharColorScheme helix;

        helix.setTextColorStyle('E', TextColorStyle(Qt::black, Qt::magenta));
        helix.setTextColorStyle('M', TextColorStyle(Qt::black, QColor(239, 16, 239)));
        helix.setTextColorStyle('A', TextColorStyle(Qt::black, QColor(231, 24, 231)));
        helix.setTextColorStyle('Z', TextColorStyle(Qt::black, QColor(201, 54, 201)));
        helix.setTextColorStyle('L', TextColorStyle(Qt::black, QColor(174, 81, 174)));
        helix.setTextColorStyle('K', TextColorStyle(Qt::black, QColor(160, 95, 160)));
        helix.setTextColorStyle('F', TextColorStyle(Qt::black, QColor(152, 103, 152)));
        helix.setTextColorStyle('Q', TextColorStyle(Qt::black, QColor(146, 109, 146)));
        helix.setTextColorStyle('I', TextColorStyle(Qt::black, QColor(138, 117, 138)));
        helix.setTextColorStyle('W', TextColorStyle(Qt::black, QColor(138, 117, 138)));
        helix.setTextColorStyle('V', TextColorStyle(Qt::black, QColor(133, 122, 133)));
        helix.setTextColorStyle('D', TextColorStyle(Qt::black, QColor(119, 136, 119)));
        helix.setTextColorStyle('X', TextColorStyle(Qt::black, QColor(117, 138, 117)));
        helix.setTextColorStyle('H', TextColorStyle(Qt::black, QColor(117, 138, 117)));
        helix.setTextColorStyle('R', TextColorStyle(Qt::black, QColor(111, 144, 111)));
        helix.setTextColorStyle('B', TextColorStyle(Qt::black, QColor(73, 182, 73)));
        helix.setTextColorStyle('T', TextColorStyle(Qt::black, QColor(71, 184, 71)));
        helix.setTextColorStyle('S', TextColorStyle(Qt::black, QColor(54, 201, 54)));
        helix.setTextColorStyle('C', TextColorStyle(Qt::black, QColor(35, 220, 35)));
        helix.setTextColorStyle('Y', TextColorStyle(Qt::black, QColor(33, 222, 33)));
        helix.setTextColorStyle('N', TextColorStyle(Qt::black, QColor(27, 228, 27)));
        helix.setTextColorStyle('G', TextColorStyle(Qt::black, Qt::green));
        helix.setTextColorStyle('P', TextColorStyle(Qt::black, Qt::green));

        return helix;
    }
    const CharColorScheme kHelixPropensityScheme(initializeHelixPropensityAminoColorScheme());

    CharColorScheme initializeStrandPropensityAminoColorScheme()
    {
        CharColorScheme strand;

        strand.setTextColorStyle('V', TextColorStyle(Qt::black, Qt::yellow));
        strand.setTextColorStyle('I', TextColorStyle(Qt::black, QColor(236, 236, 19)));
        strand.setTextColorStyle('Y', TextColorStyle(Qt::black, QColor(211, 211, 44)));
        strand.setTextColorStyle('F', TextColorStyle(Qt::black, QColor(194, 194, 61)));
        strand.setTextColorStyle('W', TextColorStyle(Qt::black, QColor(192, 192, 63)));
        strand.setTextColorStyle('L', TextColorStyle(Qt::black, QColor(178, 178, 77)));
        strand.setTextColorStyle('T', TextColorStyle(Qt::black, QColor(157, 157, 98)));
        strand.setTextColorStyle('C', TextColorStyle(Qt::black, QColor(157, 157, 98)));
        strand.setTextColorStyle('Q', TextColorStyle(Qt::black, QColor(140, 140, 115)));
        strand.setTextColorStyle('M', TextColorStyle(Qt::black, QColor(130, 130, 125)));
        strand.setTextColorStyle('X', TextColorStyle(Qt::black, QColor(121, 121, 134)));
        strand.setTextColorStyle('R', TextColorStyle(Qt::black, QColor(107, 107, 148)));
        strand.setTextColorStyle('N', TextColorStyle(Qt::black, QColor(100, 100, 155)));
        strand.setTextColorStyle('H', TextColorStyle(Qt::black, QColor(96, 96, 159)));
        strand.setTextColorStyle('A', TextColorStyle(Qt::black, QColor(88, 88, 167)));
        strand.setTextColorStyle('S', TextColorStyle(Qt::black, QColor(73, 73, 182)));
        strand.setTextColorStyle('G', TextColorStyle(Qt::black, QColor(73, 73, 182)));
        strand.setTextColorStyle('Z', TextColorStyle(Qt::black, QColor(71, 71, 184)));
        strand.setTextColorStyle('K', TextColorStyle(Qt::black, QColor(71, 71, 184)));
        strand.setTextColorStyle('B', TextColorStyle(Qt::black, QColor(67, 67, 188)));
        strand.setTextColorStyle('P', TextColorStyle(Qt::black, QColor(35, 35, 220)));
        strand.setTextColorStyle('D', TextColorStyle(Qt::black, QColor(33, 33, 222)));
        strand.setTextColorStyle('E', TextColorStyle(Qt::black, Qt::blue));

        return strand;
    }
    const CharColorScheme kStrandPropensityScheme(initializeStrandPropensityAminoColorScheme());

    CharColorScheme initializeTurnPropensityAminoColorScheme()
    {
        CharColorScheme turn;

        turn.setTextColorStyle('N', TextColorStyle(Qt::black, Qt::red));
        turn.setTextColorStyle('G', TextColorStyle(Qt::black, Qt::red));
        turn.setTextColorStyle('P', TextColorStyle(Qt::black, QColor(246, 9, 9)));
        turn.setTextColorStyle('B', TextColorStyle(Qt::black, QColor(243, 12, 12)));
        turn.setTextColorStyle('D', TextColorStyle(Qt::black, QColor(232, 23, 23)));
        turn.setTextColorStyle('S', TextColorStyle(Qt::black, QColor(225, 30, 30)));
        turn.setTextColorStyle('C', TextColorStyle(Qt::black, QColor(168, 87, 87)));
        turn.setTextColorStyle('Y', TextColorStyle(Qt::black, QColor(157, 98, 98)));
        turn.setTextColorStyle('K', TextColorStyle(Qt::black, QColor(126, 129, 129)));
        turn.setTextColorStyle('X', TextColorStyle(Qt::black, QColor(124, 131, 131)));
        turn.setTextColorStyle('Q', TextColorStyle(Qt::black, QColor(119, 136, 136)));
        turn.setTextColorStyle('W', TextColorStyle(Qt::black, QColor(115, 140, 140)));
        turn.setTextColorStyle('T', TextColorStyle(Qt::black, QColor(115, 140, 140)));
        turn.setTextColorStyle('R', TextColorStyle(Qt::black, QColor(112, 143, 143)));
        turn.setTextColorStyle('H', TextColorStyle(Qt::black, QColor(112, 143, 143)));
        turn.setTextColorStyle('Z', TextColorStyle(Qt::black, QColor(91, 164, 164)));
        turn.setTextColorStyle('E', TextColorStyle(Qt::black, QColor(63, 192, 192)));
        turn.setTextColorStyle('A', TextColorStyle(Qt::black, QColor(44, 211, 211)));
        turn.setTextColorStyle('F', TextColorStyle(Qt::black, QColor(30, 225, 225)));
        turn.setTextColorStyle('M', TextColorStyle(Qt::black, QColor(30, 225, 225)));
        turn.setTextColorStyle('L', TextColorStyle(Qt::black, QColor(28, 227, 227)));
        turn.setTextColorStyle('V', TextColorStyle(Qt::black, QColor(7, 248, 248)));
        turn.setTextColorStyle('I', TextColorStyle(Qt::black, QColor(0, 255, 255)));

        return turn;
    }
    const CharColorScheme kTurnPropensityScheme(initializeTurnPropensityAminoColorScheme());

    CharColorScheme initializeBuriedIndexColorScheme()
    {
        CharColorScheme buried;

        buried.setTextColorStyle('C', TextColorStyle(Qt::black, Qt::blue));
        buried.setTextColorStyle('I', TextColorStyle(Qt::black, QColor(0, 84, 171)));
        buried.setTextColorStyle('V', TextColorStyle(Qt::black, QColor(0, 95, 160)));
        buried.setTextColorStyle('L', TextColorStyle(Qt::black, QColor(0, 123, 132)));
        buried.setTextColorStyle('F', TextColorStyle(Qt::black, QColor(0, 135, 120)));
        buried.setTextColorStyle('M', TextColorStyle(Qt::black, QColor(0, 151, 104)));
        buried.setTextColorStyle('G', TextColorStyle(Qt::black, QColor(0, 157, 98)));
        buried.setTextColorStyle('A', TextColorStyle(Qt::black, QColor(0, 163, 92)));
        buried.setTextColorStyle('W', TextColorStyle(Qt::black, QColor(0, 168, 87)));
        buried.setTextColorStyle('X', TextColorStyle(Qt::black, QColor(0, 182, 73)));
        buried.setTextColorStyle('S', TextColorStyle(Qt::black, QColor(0, 213, 42)));
        buried.setTextColorStyle('H', TextColorStyle(Qt::black, QColor(0, 213, 42)));
        buried.setTextColorStyle('T', TextColorStyle(Qt::black, QColor(0, 219, 36)));
        buried.setTextColorStyle('P', TextColorStyle(Qt::black, QColor(0, 224, 31)));
        buried.setTextColorStyle('Y', TextColorStyle(Qt::black, QColor(0, 230, 25)));
        buried.setTextColorStyle('N', TextColorStyle(Qt::black, QColor(0, 235, 20)));
        buried.setTextColorStyle('B', TextColorStyle(Qt::black, QColor(0, 235, 20)));
        buried.setTextColorStyle('D', TextColorStyle(Qt::black, QColor(0, 235, 20)));
        buried.setTextColorStyle('Q', TextColorStyle(Qt::black, QColor(0, 241, 14)));
        buried.setTextColorStyle('Z', TextColorStyle(Qt::black, QColor(0, 241, 14)));
        buried.setTextColorStyle('E', TextColorStyle(Qt::black, QColor(0, 241, 14)));
        buried.setTextColorStyle('R', TextColorStyle(Qt::black, QColor(0, 252, 3)));
        buried.setTextColorStyle('K', TextColorStyle(Qt::black, QColor(0, 255, 0)));

        return buried;
    }
    const CharColorScheme kBuriedIndexScheme(initializeBuriedIndexColorScheme());




    LinearColorScheme initializeSecondaryStructureColorScheme()
    {
        LinearColorScheme secondary;

        QColor white(255, 255, 255);
        QColor black(0, 0, 0);
        QColor red(255, 0, 0);
        QColor blue(0, 96, 255);

        secondary.setBackgroundLinearColors('H', LinearColorStyle(PairQColor(white, red), 10));
        secondary.setBackgroundLinearColors('E', LinearColorStyle(PairQColor(white, blue), 10));
        secondary.setForegroundLinearColors('L', LinearColorStyle(PairQColor(white, black), 10));

        return secondary;
    }
    const LinearColorScheme kSecondaryStructureScheme(initializeSecondaryStructureColorScheme());




    CharColorScheme initializeClustalDnaColorScheme()
    {
        CharColorScheme clustal;

        clustal.setTextColorStyle('A', TextColorStyle(Qt::black, QColor(100, 247, 63)));
        clustal.setTextColorStyle('T', TextColorStyle(Qt::black, QColor(60, 136, 238)));
        clustal.setTextColorStyle('C', TextColorStyle(Qt::black, QColor(255, 179, 64)));
        clustal.setTextColorStyle('G', TextColorStyle(Qt::black, QColor(235, 65, 60)));

        return clustal;
    }
    const CharColorScheme kClustalDnaScheme(initializeClustalDnaColorScheme());

    CharColorScheme initializeLogoAminoScheme()
    {
        CharColorScheme logo;

        QColor green(0, 204, 0);
        QColor blue(0, 0, 204);
        QColor red(204, 0, 0);
        QColor purple(204, 0, 204);

        logo.setTextColorStyle('S', TextColorStyle(green));
        logo.setTextColorStyle('T', TextColorStyle(green));
        logo.setTextColorStyle('Y', TextColorStyle(green));
        logo.setTextColorStyle('G', TextColorStyle(green));

        logo.setTextColorStyle('Q', TextColorStyle(purple));
        logo.setTextColorStyle('N', TextColorStyle(purple));

        logo.setTextColorStyle('D', TextColorStyle(red));
        logo.setTextColorStyle('E', TextColorStyle(red));

        logo.setTextColorStyle('K', TextColorStyle(blue));
        logo.setTextColorStyle('R', TextColorStyle(blue));

        return logo;
    }
    const CharColorScheme kLogoAminoScheme(initializeLogoAminoScheme());

//    .setTextColorStyle('', TextColorStyle(Qt::black, QColor()));
}
