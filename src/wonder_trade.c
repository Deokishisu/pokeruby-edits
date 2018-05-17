#include "global.h"
#include "pokemon.h"
#include "strings2.h"
#include "random.h"
#include "text.h"
#include "event_data.h"
#include "region_map.h"
#include "constants/species.h"
#include "constants/items.h"
#include "constants/abilities.h"
#include "data/text/wonder_trade_OT_names.h"

struct InGameTrade {
    /*0x00*/ u8 name[11];
    /*0x0C*/ u16 species;
    /*0x0E*/ u8 ivs[6];
    /*0x14*/ bool8 secondAbility;
    /*0x18*/ u32 otId;
    /*0x1C*/ u8 stats[5];
    /*0x24*/ u32 personality;
    /*0x28*/ u16 heldItem;
    /*0x2A*/ u8 mailNum;
    /*0x2B*/ u8 otName[11];
    /*0x36*/ u8 otGender;
    /*0x37*/ u8 sheen;
    /*0x38*/ u16 playerSpecies;
};

static u16 PickRandomSpecies(void);
void CreateWonderTradePokemon(u8, u8);
u16 returnValidSpecies(u16);
u16 determineEvolution(struct Pokemon *);
u8 getWonderTradeOT(u8 *);

static u16 PickRandomSpecies() //picks only base forms
{
	u16 species = Random() % 179;
	species += 1;
	species = returnValidSpecies(species);
	return species;
}

u8 getWonderTradeOT(u8 *name)
{
	u8 randGender = (Random() % 2); //0 for male, 1 for female
	u8 numOTNames = 250;
	u8 selectedName = Random() %numOTNames;
	u8 i;
	if(randGender == MALE) //male OT selected
	{
		randGender = 0;
		for(i = 0; i < 8; ++i)
		{
			name[i] = maleWTNames[selectedName][i];
		}
		name[8] = EOS;
	}
	else				//female OT selected
	{
		randGender = 0xFF;
		for(i = 0; i < 8; ++i)
		{
			name[i] = femaleWTNames[selectedName][i];
		}
		name[8] = EOS;
	}
	return randGender;
}

void CreateWonderTradePokemon(u8 whichPlayerMon, u8 whichInGameTrade)
{
	struct InGameTrade *inGameTrade;
    u16 species = PickRandomSpecies();
    u8 chanceToEvolve = Random() % 256;
    //u8 metLevel = 0;
    //u8 gameOrigin = VERSION_RUBY;
    u8 name[POKEMON_NAME_LENGTH + 1];
    u8 nameOT[8];
    u8 genderOT;
	u8 level = GetMonData(&gPlayerParty[gSpecialVar_0x800A], MON_DATA_LEVEL); //gets level of player's selected Pokemon
	u16 playerSpecies = GetMonData(&gPlayerParty[gSpecialVar_0x800A], MON_DATA_SPECIES); //gets species of player's selected Pokemon
	u32 OTID = ((Random() << 16) | Random());
	u32 personality = ((Random() << 16) | Random());
	struct Pokemon *pokemon = &gEnemyParty[0];
    u8 metLocation = 0xFE;
    struct InGameTrade gIngameTrades[] = {
        {
        	_(""), species,
			(Random() % 32), (Random() % 32), (Random() % 32), (Random() % 32), (Random() % 32), (Random() % 32),
			(Random() % 2), OTID,
            0, 0, 0, 0, 0,
            personality,
            ITEM_NONE, -1,
            _("ERROR"), FEMALE, 0,
            playerSpecies
        }
    };
    GetSpeciesName(name, species);
    genderOT = getWonderTradeOT(nameOT);

    inGameTrade = &gIngameTrades[0];

    CreateMon(pokemon, species, level, 0, FALSE, 0, TRUE, (Random() << 16) | Random());
    if (chanceToEvolve >= 15) //evolves to highest stage
    {
    	species = determineEvolution(pokemon);
    	CreateMon(pokemon, species, level, 0, FALSE, 0, TRUE, (Random() << 16) | Random());
    	species = determineEvolution(pokemon);
    }
    else if (chanceToEvolve > 7 && chanceToEvolve < 15) //evolves once
    {
    	species = determineEvolution(pokemon);
    }
    GetSpeciesName(name, species);
    if(gBaseStats[species].ability2 == ABILITY_NONE) //if species has no second ability, set first ability
    {
    	inGameTrade->secondAbility = 0;
    }

    CreateMon(pokemon, species, level, 0, FALSE, 0, TRUE, (Random() << 16) | Random());
    SetMonData(pokemon, MON_DATA_HP_IV, &inGameTrade->ivs[0]);
    SetMonData(pokemon, MON_DATA_ATK_IV, &inGameTrade->ivs[1]);
    SetMonData(pokemon, MON_DATA_DEF_IV, &inGameTrade->ivs[2]);
    SetMonData(pokemon, MON_DATA_SPEED_IV, &inGameTrade->ivs[3]);
    SetMonData(pokemon, MON_DATA_SPATK_IV, &inGameTrade->ivs[4]);
    SetMonData(pokemon, MON_DATA_SPDEF_IV, &inGameTrade->ivs[5]);
    SetMonData(pokemon, MON_DATA_NICKNAME, name);
    SetMonData(pokemon, MON_DATA_OT_NAME, nameOT);
    SetMonData(pokemon, MON_DATA_ALT_ABILITY, &inGameTrade->secondAbility);
    SetMonData(pokemon, MON_DATA_BEAUTY, &inGameTrade->stats[1]);
    SetMonData(pokemon, MON_DATA_CUTE, &inGameTrade->stats[2]);
    SetMonData(pokemon, MON_DATA_COOL, &inGameTrade->stats[0]);
    SetMonData(pokemon, MON_DATA_SMART, &inGameTrade->stats[3]);
    SetMonData(pokemon, MON_DATA_TOUGH, &inGameTrade->stats[4]);
    SetMonData(pokemon, MON_DATA_SHEEN, &inGameTrade->sheen);
    SetMonData(pokemon, MON_DATA_MET_LOCATION, &metLocation);
    SetMonData(pokemon, MON_DATA_OT_GENDER, genderOT);
    CalculateMonStats(&gEnemyParty[0]);
}

u16 determineEvolution(struct Pokemon *mon)
{
	int i;
	u16 targetSpecies = 0;
	u16 species = GetMonData(mon, MON_DATA_SPECIES, 0);
	u32 personality = GetMonData(mon, MON_DATA_PERSONALITY, 0);
	u16 upperPersonality = personality >> 16;
	u8 level = GetMonData(mon, MON_DATA_LEVEL, 0);
	u16 eeveelution = Random() % 5;

	if(species == SPECIES_EEVEE || species == SPECIES_NINCADA)
	{
		if(species == SPECIES_NINCADA && level >= 20)
    	{
    		if((Random() % 2) == 0)
    			targetSpecies = SPECIES_NINJASK;
    		else
    			targetSpecies = SPECIES_SHEDINJA;
    		return targetSpecies;
    	}
		else if(species == SPECIES_EEVEE && level >= 25)
    	{
    		if(eeveelution == 0)
    			targetSpecies = SPECIES_VAPOREON;
    		else if(eeveelution == 1)
    			targetSpecies = SPECIES_JOLTEON;
    		else if(eeveelution == 2)
    			targetSpecies = SPECIES_FLAREON;
    		else if(eeveelution == 3)
    			targetSpecies = SPECIES_ESPEON;
    		else if(eeveelution == 4)
    			targetSpecies = SPECIES_UMBREON;
    		return targetSpecies;
    	}
		else //not the right level
			return targetSpecies;
	}

	for (i = 0; i < 5; i++)
	{
		switch (gEvolutionTable[species][i].method)
	    {
	    case EVO_FRIENDSHIP:
	    	if ((species == SPECIES_PICHU || species == SPECIES_CLEFFA || species == SPECIES_IGGLYBUFF
	    		|| species == SPECIES_TOGEPI || species == SPECIES_AZURILL) && level >= 16)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if((species == SPECIES_GOLBAT || species == SPECIES_CHANSEY) && level >= 35)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else
	    		break;
	        break;
	    case EVO_LEVEL:
	    	if (species == SPECIES_SLOWPOKE && level >= 37)
	    	{
	    		if((Random() % 2) == 0)
	    			targetSpecies = SPECIES_SLOWBRO;
	    		else
	    			targetSpecies = SPECIES_SLOWKING;
	    	}
	    	else if (gEvolutionTable[species][i].param <= level)
	         	targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else
	    		break;
	        break;
	    case EVO_LEVEL_ATK_GT_DEF:
	     	if (gEvolutionTable[species][i].param <= level)
	        	if (GetMonData(mon, MON_DATA_ATK, 0) > GetMonData(mon, MON_DATA_DEF, 0))
	            	targetSpecies = gEvolutionTable[species][i].targetSpecies;
	        break;
	    case EVO_LEVEL_ATK_EQ_DEF:
	     	if (gEvolutionTable[species][i].param <= level)
	        	if (GetMonData(mon, MON_DATA_ATK, 0) == GetMonData(mon, MON_DATA_DEF, 0))
	            	targetSpecies = gEvolutionTable[species][i].targetSpecies;
	        break;
	    case EVO_LEVEL_ATK_LT_DEF:
	     	if (gEvolutionTable[species][i].param <= level)
	        	if (GetMonData(mon, MON_DATA_ATK, 0) < GetMonData(mon, MON_DATA_DEF, 0))
	            	targetSpecies = gEvolutionTable[species][i].targetSpecies;
	        break;
	    case EVO_LEVEL_SILCOON:
	     	if (gEvolutionTable[species][i].param <= level && (upperPersonality % 10) <= 4)
	     		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	        break;
	    case EVO_LEVEL_CASCOON:
	    	if (gEvolutionTable[species][i].param <= level && (upperPersonality % 10) > 4)
	         	targetSpecies = gEvolutionTable[species][i].targetSpecies;
	     	break;
	    case EVO_BEAUTY:
	     	if (level >= 30)
	        	targetSpecies = gEvolutionTable[species][i].targetSpecies;
	        break;
	    case EVO_ITEM:
	    	if (species == SPECIES_GLOOM && level >= 36)
	    	{
	    		if((Random() % 2) == 0)
	    			targetSpecies = SPECIES_VILEPLUME;
	    		else
	    			targetSpecies = SPECIES_BELLOSSOM;
	    	}
	    	else if (species == SPECIES_WEEPINBELL && level >= 36)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if ((species == SPECIES_VULPIX || species == SPECIES_GROWLITHE) && level >= 32)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if ((species == SPECIES_SHELLDER || species == SPECIES_STARYU) && level >= 43)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if((species == SPECIES_NIDORINA || species == SPECIES_NIDORINO || species == SPECIES_EXEGGCUTE) && level >= 26)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if((species == SPECIES_JIGGLYPUFF || species == SPECIES_CLEFAIRY || species == SPECIES_SKITTY) && level >= 38)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if((species == SPECIES_LOMBRE || species == SPECIES_NUZLEAF) && level >= 38)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if(species == SPECIES_POLIWHIRL && level >= 44)
	    	{
	    		if((Random() % 2) == 0)
	    			targetSpecies = SPECIES_POLIWRATH;
	    		else
	    			targetSpecies = SPECIES_POLITOED;
	    	}
	    	else if(species == SPECIES_PIKACHU && level >= 27)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if(species == SPECIES_SUNKERN && level >= 15)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else
	    		break;
	    	break;
	    case EVO_TRADE_ITEM:
	    	if ((species == SPECIES_ONIX || species == SPECIES_SEADRA) && level >= 40)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if (species == SPECIES_SCYTHER && level >= 26)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if (species == SPECIES_PORYGON && level >= 21)
	    		targetSpecies = gEvolutionTable[species][i].targetSpecies;
	    	else if (species == SPECIES_CLAMPERL && level >= 22)
	    	{
	    		if((Random() % 2) == 0)
	    			targetSpecies = SPECIES_HUNTAIL;
	    		else
	    			targetSpecies = SPECIES_GOREBYSS;
	    	}
	    	else
	    		break;
	    	break;
	    }
	}
		if(targetSpecies == 0)
			return species;
		else
			return targetSpecies;
}

u16 returnValidSpecies(u16 input)
{
	u16 species[] =
	{
			SPECIES_BULBASAUR,
			SPECIES_CHARMANDER,
			SPECIES_SQUIRTLE,
			SPECIES_CATERPIE,
			SPECIES_WEEDLE,
			SPECIES_PIDGEY,
			SPECIES_RATTATA,
			SPECIES_SPEAROW,
			SPECIES_EKANS,
			SPECIES_SANDSHREW,
			SPECIES_NIDORAN_F,
			SPECIES_NIDORAN_M,
			SPECIES_VULPIX,
			SPECIES_ZUBAT,
			SPECIES_ODDISH,
			SPECIES_PARAS,
			SPECIES_VENONAT,
			SPECIES_DIGLETT,
			SPECIES_MEOWTH,
			SPECIES_PSYDUCK, //20
			SPECIES_MANKEY,
			SPECIES_GROWLITHE,
			SPECIES_POLIWAG,
			SPECIES_ABRA,
			SPECIES_MACHOP,
			SPECIES_BELLSPROUT,
			SPECIES_TENTACOOL,
			SPECIES_GEODUDE,
			SPECIES_PONYTA,
			SPECIES_SLOWPOKE,
			SPECIES_MAGNEMITE,
			SPECIES_FARFETCHD,
			SPECIES_DODUO,
			SPECIES_SEEL,
			SPECIES_GRIMER,
			SPECIES_SHELLDER,
			SPECIES_GASTLY,
			SPECIES_ONIX,
			SPECIES_DROWZEE,
			SPECIES_KRABBY, //40
			SPECIES_VOLTORB,
			SPECIES_EXEGGCUTE,
			SPECIES_CUBONE,
			SPECIES_LICKITUNG,
			SPECIES_KOFFING,
			SPECIES_RHYHORN,
			SPECIES_CHANSEY,
			SPECIES_TANGELA,
			SPECIES_KANGASKHAN,
			SPECIES_HORSEA,
			SPECIES_GOLDEEN,
			SPECIES_STARYU,
			SPECIES_MR_MIME,
			SPECIES_SCYTHER,
			SPECIES_PINSIR,
			SPECIES_TAUROS,
			SPECIES_MAGIKARP,
			SPECIES_LAPRAS,
			SPECIES_DITTO,
			SPECIES_EEVEE, //60
			SPECIES_PORYGON,
			SPECIES_OMANYTE,
			SPECIES_KABUTO,
			SPECIES_AERODACTYL,
			SPECIES_SNORLAX,
			SPECIES_DRATINI,
			SPECIES_CHIKORITA,
			SPECIES_TOTODILE,
			SPECIES_SENTRET,
			SPECIES_HOOTHOOT,
			SPECIES_LEDYBA,
			SPECIES_SPINARAK,
			SPECIES_CHINCHOU,
			SPECIES_PICHU,
			SPECIES_CLEFFA,
			SPECIES_IGGLYBUFF,
			SPECIES_TOGEPI,
			SPECIES_NATU,
			SPECIES_MAREEP,
			SPECIES_SUDOWOODO, //80
			SPECIES_HOPPIP,
			SPECIES_AIPOM,
			SPECIES_SUNKERN,
			SPECIES_YANMA,
			SPECIES_WOOPER,
			SPECIES_MURKROW,
			SPECIES_MISDREAVUS,
			SPECIES_UNOWN,
			SPECIES_GIRAFARIG,
			SPECIES_PINECO,
			SPECIES_DUNSPARCE,
			SPECIES_GLIGAR,
			SPECIES_SNUBBULL,
			SPECIES_QWILFISH,
			SPECIES_SHUCKLE,
			SPECIES_HERACROSS,
			SPECIES_SNEASEL,
			SPECIES_TEDDIURSA,
			SPECIES_SLUGMA,
			SPECIES_SWINUB, //100
			SPECIES_CORSOLA,
			SPECIES_REMORAID,
			SPECIES_DELIBIRD,
			SPECIES_MANTINE,
			SPECIES_SKARMORY,
			SPECIES_HOUNDOUR,
			SPECIES_PHANPY,
			SPECIES_STANTLER,
			SPECIES_SMEARGLE,
			SPECIES_TYROGUE,
			SPECIES_SMOOCHUM,
			SPECIES_ELEKID,
			SPECIES_MAGBY,
			SPECIES_MILTANK,
			SPECIES_LARVITAR,
			SPECIES_TREECKO,
			SPECIES_TORCHIC,
			SPECIES_MUDKIP,
			SPECIES_POOCHYENA,
			SPECIES_ZIGZAGOON, //120
			SPECIES_WURMPLE,
			SPECIES_LOTAD,
			SPECIES_SEEDOT,
			SPECIES_NINCADA,
			SPECIES_TAILLOW,
			SPECIES_SHROOMISH,
			SPECIES_SPINDA,
			SPECIES_WINGULL,
			SPECIES_SURSKIT,
			SPECIES_WAILMER,
			SPECIES_SKITTY,
			SPECIES_KECLEON,
			SPECIES_BALTOY,
			SPECIES_NOSEPASS,
			SPECIES_TORKOAL,
			SPECIES_SABLEYE,
			SPECIES_BARBOACH,
			SPECIES_LUVDISC,
			SPECIES_CORPHISH,
			SPECIES_FEEBAS, //140
			SPECIES_CARVANHA,
			SPECIES_TRAPINCH,
			SPECIES_MAKUHITA,
			SPECIES_ELECTRIKE,
			SPECIES_NUMEL,
			SPECIES_SPHEAL,
			SPECIES_CACNEA,
			SPECIES_SNORUNT,
			SPECIES_LUNATONE,
			SPECIES_SOLROCK,
			SPECIES_AZURILL,
			SPECIES_SPOINK,
			SPECIES_PLUSLE,
			SPECIES_MINUN,
			SPECIES_MAWILE,
			SPECIES_MEDITITE,
			SPECIES_SWABLU,
			SPECIES_WYNAUT,
			SPECIES_DUSKULL,
			SPECIES_ROSELIA, //160
			SPECIES_SLAKOTH,
			SPECIES_GULPIN,
			SPECIES_TROPIUS,
			SPECIES_WHISMUR,
			SPECIES_CLAMPERL,
			SPECIES_ABSOL,
			SPECIES_SHUPPET,
			SPECIES_SEVIPER,
			SPECIES_ZANGOOSE,
			SPECIES_RELICANTH,
			SPECIES_ARON,
			SPECIES_CASTFORM,
			SPECIES_VOLBEAT,
			SPECIES_ILLUMISE,
			SPECIES_LILEEP,
			SPECIES_ANORITH,
			SPECIES_RALTS,
			SPECIES_BAGON,
			SPECIES_BELDUM,
			SPECIES_CHIMECHO //180
	};
	return species[input];
}
