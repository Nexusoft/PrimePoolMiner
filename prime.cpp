/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn, Create, but do not Forge] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/

#include "core.h"
#include <map>

#include "oacc/AccSieve.h"
using namespace std;


namespace Core
{
	uint32_t *primes;
	uint32_t *inverses;

	unsigned int nBitArray_Size =  1024*1024*8;
	mpz_t  zPrimorial;

	unsigned int prime_limit = 71378571;
	unsigned int nPrimeLimit = 16384 * 96;
	unsigned int nPrimorialEndPrime = 12;

	
	uint64 octuplet_origins[256]  = {15760091,25658441,93625991,182403491,226449521,661972301,910935911,1042090781,1071322781,1170221861,1394025161,1459270271,1712750771,
											1742638811,1935587651,2048038451,2397437501,2799645461,2843348351,3734403131,4090833821,5349522791,5379039551,5522988461,5794564661,
											5950513181,6070429481,6138646511,6193303001,6394117181,6520678511,6765896981,6969026411,7219975571,7602979451,8247812381,8750853101,
											9870884321,9966184841,11076719651,11234903411,11567910701,11881131791,12753314921,12848960471,12850665671,12886759001,13345214411,
											13421076281,15065117141,15821203241,16206106991,16427277941,16804790531,17140322651,17383048211,18234075311,18379278761,18821455181,
											18856092371,21276989801,21315831611,21803245811,22190786531,22367332061,22642418411,22784826131,22827253901,23393094071,24816950771,
											24887046251,24930296381,26092031081,28657304561,28900195391,29055481571,29906747861,30332927741,30526543121,31282661141,31437430091,
											31447680611,31779849371,31907755331,33081664151,33734375021,35035293101,35969034371,36551720741,37000821701,37037888801,37654490171,
											38855298941,40743911051,41614070411,43298074271,43813839521,44676352991,45549998561,46961199401,47346763811,48333938111,49788942011,
											49827604901,50144961941,50878435451,53001578081,54270148391,57440594201,60239937671,62184803951,63370318001,64202502431,65227645781,
											65409385031,66449431661,69707273171,71750241371,73457668631,74082349331,74445418121,74760009671,75161088461,75778477121,76289638961,
											77310104141,77653734071,78065091101,78525462131,79011826961,79863776801,79976720891,80041993301,80587471031,80790462281,82455937631,
											83122625471,84748266131,84882447101,85544974631,86408384591,87072248561,88163200661,88436579501,88815669401,89597692181,90103909781,
											91192669481,93288681371,93434383571,93487652171,93703549391,94943708591,95109448781,95391400451,96133393241,97249028951,98257943081,
											100196170421,101698684931,104487717401,105510861341,106506834431,107086217081,109750518791,110327129441,111422173391,114994357391,
											116632573901,117762315941,118025332961,119063726051,121317512201,123019590761,123775576271,124168028051,130683361421,131045869301,
											131176761251,131484693071,132595345691,133391614241,135614688941,138478375151,139017478331,139858746941,141763537451,143258671091,
											144224334251,147215781521,147332222951,148124799281,148323246341,148671287111,148719488831,148916953301,148949723381,150613299911,
											153779378561,155130467951,155521458551,156146394401,156456267881,157272782741,157519407581,163899228791,164138756051,165040690931,
											165792941381,165952761041,166004527301,166225007561,168626248781,169349651741,170316751721,170552481551,170587733201,170832928151,
											171681030791,172892544941,173405293331,174073117061,177620195561,178242755681,180180782051,180237252311,184430028311,185515423391,
											185814366581,186122739611,187735172741,187971393341,188090847011,189066712181,190192014821,192380171981,193725710021,194875423271,
											198006027671,198146724311,198658763111,198869317721,199658510321,199847262731,200599766441,201708760061,202506276431,203499800501,
											204503641871,206150764271,207369666851,208403006081,211925962091,214556015741,218389714001,218732226521};

	//uint64 tentuplet2_origins[] = { 941649232615361,941649232615361,941649232615361,941649232615361,941649232615361,941649232615361,941649232615361,941649232615361,941649232615361,941649232615361 };

	uint64 tentuplet2_origins[] = { 7908189600581, 10527733922591, 12640876669691, 38545620633251, 43564522846961, 60268613366231, 60596839933361, 71431649320301, 79405799458871, 109319665100531, 153467532929981, 171316998238271, 216585060731771, 254583955361621, 259685796605351, 268349524548221, 290857309443971, 295606138063121, 380284918609481, 437163765888581, 440801576660561, 487925557660811, 504725363988731, 520113801406931, 595207784226941, 625281485054111, 631378766940191, 673792122184451, 681046359388511, 689316151753961, 695207112224741, 701889794782061, 711849170091791, 715567045378301, 734567711422661, 745427831287871, 795308572603661, 868682314445831, 869314742861681, 871195311482381, 941649232615361,
		33081664151,83122625471,294920291201,573459229151,663903555851,688697679401,730121110331,1044815397161,1089869189021,1108671297731,1235039237891,1291458592421,1738278660731,1761428046221,1769102630411,1804037746781,1944541048181,2135766611591,2177961410741,2206701370871,2395426439291,2472430065221,2601621164951,2690705555381,2744825921681,2745579799421,2772177619481,2902036214921,3132826508081,3447850801511,3640652828621,3692145722801,4136896626671,4360784021591,4563595602101,4582294051871,4700094892301,5289415841441,5308007072981,5351833972601,5731733114951,5912642968691,5923626073901,6218504101541,7353767766461,7535450701391,7559909355611,9062538296081,9494258167391,9898228819091,11808683662661,12424633023611,12467997477821,12471156096011,12637149919391,12661770282431,12941332539251,13083931936961,13177285036031,13281538177751,13474617984671,13564397742431,14214158076731,14528577634841,14935307623121,15257534367611,15771001151441,15948208421681,16647704227151,17267454870551,17365931298251,17813681998571,17938517636771,18091735543151,18947646459011,19538901887621,19577485120001,20054859604511,20097538393391,20516952834221,20790089458751,20933920839101,21807698163521,21939572224301,23280376374371,23960929422161,25419097742651,25923673657151,25965833629271,26019670276331,26101057210841,26788150277501,27305074486421,27809177299901,28027335501701,28058118894341,28609991312711,29215723742321,29321817997931,29415143182961,29734781696351,30066514250231,30306643155401,30491978649941,32031885520751,32077660213211,32078946815801,32177891258321,32195059349261,32389598962991,32685957713021,32768010337871,32834252076161,33257396547491,33276520972811,33553699164521,33922370507141,35218098176531,36238457306321,36324444201581,36340570852121,36608331995351,36671136130241,36683401486001,36822642941081,36884191543931,37824019474511,38029448791331,39527072005691,39800828518721,40787955947351,40865589988031,41793740194091,42543399451361,43063568783771,43443517990331,44303507303171,45436752056231,45483573547871,46461061347971,46678045878461,46902845784911,46950720918371,48809302182911,48973645093181,49249629233921,50164585605131,51819088949471 };

	uint64 a13tuplet_origins[] = { 7933248530182091ULL, 20475715985020181ULL, 21817283854511261ULL, 33502273017038711ULL, 40257009922154141ULL, 49242777550551701ULL, 49600456951571411ULL, 75093141517810301ULL, 84653373093824651ULL, 119308586807395871ULL, 129037438367672951ULL, 129706953139869221ULL, 151242381725881331ULL, 158947009165390331ULL, 161216594737343261ULL, 167317340088093311ULL, 176587730173540571ULL, 178444395317213141ULL, 197053322268438521ULL, 301854920123441801ULL, 410741703838050701ULL, 418640642375881811ULL, 423977804750405021ULL, 438814778374548461ULL, 513938108909431091ULL, 559182483598954331ULL, 564691499059653851ULL, 572448344464521641ULL, 589634360855545121ULL, 667572186219984161ULL, 691919312280151841ULL, 732472975494505361ULL, 767917069527663941ULL, 773765497922271971ULL, 787995321974806061ULL, 812625895169874671ULL, 823206647854907201ULL, 826020289994295281ULL, 841262446570150721ULL, 912908744164643111ULL, 953243559107805851ULL, 961342194334297661ULL, 967934530204757231ULL, 1006587882969594041ULL, 1011183292256483231ULL, 1146667694047021031ULL, 1167796280524278521ULL, 1183038120860290811ULL, 1209681051625792631ULL, 1261124760662461811ULL, 1429182723758225321ULL, 1438706125151270861ULL, 1530397981778941391ULL, 1599294858270965741ULL, 1627942734221619701ULL, 1652123725369352351ULL, 1714532248479805871ULL, 1854281958725682371ULL, 1927684478822007791ULL, 1951731807361120901ULL, 1973917160698558961ULL, 2009848558728626501ULL, 2173308288597141581ULL, 2272422935292885491ULL, 2278244071680632171ULL, 2341619987226845441ULL, 2343120040039944281ULL, 2365201889521345991ULL, 2456691069147179981ULL, 2483747301738328511ULL, 2638346840383620251ULL, 2682317478727872101ULL, 2682372491413700201ULL, 2697866408683491161ULL, 2706031853515125251ULL, 2714563183649972771ULL, 2988571533422069711ULL, 3009217231619434721ULL, 3040569903950274581ULL, 3165151604874374231ULL, 3167591756547604271ULL, 3237303168778044251ULL, 3255458630925065681ULL, 3405352702935168101ULL, 3507774163278650381ULL, 3535344092704149611ULL, 3551166931457610581ULL, 3566350920603506921ULL, 3625307100163064201ULL, 3729730321107937031ULL, 3822655227927291641ULL, 3861308153681033171ULL, 4031936803816506701ULL, 4191752640441161681ULL, 4192548731244568811ULL, 4318332681848930861ULL, 4419966415439540081ULL, 4608564491163618101ULL, 4855624173160656011ULL, 4946108577253299341ULL, 5004013600662332141ULL, 5009128141636113611ULL, 5237012345364234551ULL, 5342311333178075141ULL, 5375295377935688321ULL, 5390125348017985691ULL, 5481332507478002291ULL, 5554263153841958441ULL, 5554868310193468151ULL, 5647936541529717551ULL, 5686939357428214001ULL, 5718462475332142421ULL, 5815180284779341361ULL, 5976164202306974561ULL, 6015907930528441061ULL, 6114009625327278791ULL, 6158433201181517141ULL, 6333797731632426941ULL, 6395651777987817461ULL, 6420975769793670971ULL, 6688508709613586741ULL, 6732104937121609931ULL, 7126352574372296381ULL, 7128184741845403121ULL, 7141651301548859981ULL, 7200673740422857811ULL, 7448579068883428361ULL, 7490066291532539831ULL, 7717551078717245651ULL, 7796191547933484581ULL, 7993822923596334941ULL, 8047391955869201051ULL, 8096841447776806841ULL, 8142680509981272371ULL, 8172936673958921981ULL, 8260202105585015471ULL, 8417588704283935061ULL, 8444208357610201061ULL, 8681936548475985911ULL, 8705497065358018811ULL, 8718425083363780901ULL, 8868082252489515941ULL, 8936672511075525101ULL, 8942435641714130441ULL, 9088347008465620781ULL, 9248966791533905321ULL, 9285579680626415051ULL, 9289410144748839221ULL, 9295920773602948601ULL, 9556359435868791431ULL, 9589101086077418021ULL, 9752659358061105371ULL, 9831024675664517441ULL, 9883619791069640501ULL, 9909538323760400441ULL, 10138452552101909921ULL, 10170371594471514311ULL, 10327262692092542081ULL, 10507556146919867921ULL, 10570197338486911241ULL, 10688546378467152311ULL, 10883528992824613241ULL, 10923754124049321671ULL, 10937790142602859421ULL, 11010369846356665901ULL, 11074659430273774451ULL, 11088326350945458731ULL, 11102093089272631331ULL, 11312458227678553061ULL, 11451633007692123131ULL, 11473517342470328261ULL, 11566832340556308701ULL, 11704787622527225111ULL, 11864234860023131471ULL, 12040468439440346021ULL, 12060132444969714221ULL, 12109610766126809051ULL, 12133432915975656401ULL, 12281976862150420541ULL, 12283184697373485011ULL, 12484308332943792461ULL, 12658575037462640171ULL, 12687940090632041351ULL, 12743805720981340121ULL, 12790060519079219261ULL, 12826365804581023811ULL, 12856398922752490991ULL, 12861151241738035121ULL, 12862507970199143141ULL, 12870536149631655611ULL, 12906371481473675321ULL, 12950968854480722951ULL, 12980511790060529831ULL, 13326939721942032071ULL, 13647309375492208301ULL, 13825299795938817071ULL, 14076523913088511991ULL, 14249372854719388271ULL };

	uint64 a14tuplet_origins[] = { 21817283854511261ULL, 841262446570150721ULL, 1006587882969594041ULL, 2682372491413700201ULL, 5009128141636113611ULL, 7126352574372296381ULL, 7993822923596334941ULL, 12870536149631655611ULL, 15762479629138737611ULL };
		
										  // 0  1   2   3   4   5   6   7   8   9  10  11  12  13
	static uint32_t _offsets14Tuple1[14] = { 0, 2,  6,  8, 12, 18, 20, 26, 30, 32, 36, 42, 48, 50 };
	static uint32_t _offsets15Tuple1[15] = { 0, 2,  6,  8, 12, 18, 20, 26, 30, 32, 36, 42, 48, 50, 56};
	static uint32_t _offsets15Tuple2[15] = { 0, 2,  6, 12, 14, 20, 24, 26, 30, 36, 42, 44, 50, 54, 56 };

										 // 0  1   2   3   4   5   6   7   8   9  10  11  12  13  14
										 // 0, 2,  6, 12, 14, 20, 26, 30, 32, 36, 42, 44, 50, 54, 56
	static uint32_t _offsets15Tuple3[15] = { 0, 2,  6, 12, 14, 20, 26, 30, 32, 36, 42, 44, 50, 54, 56 };
	static uint32_t _offsets16Tuple3[16] = { 0, 2,  6, 12, 14, 20, 26, 30, 32, 36, 42, 44, 50, 54, 56, 60 };	
	static uint32_t _offsets19Tuple1[19] = { 0, 6, 10, 16, 18, 22, 28, 30, 36, 42, 46, 48, 52, 58, 60, 66, 70, 72, 76 }; //293281 + 510510 * n
	static uint32_t _offsets19Tuple2[19] = { 0, 4,  6, 10, 16, 18, 24, 28, 30, 34, 40, 46, 48, 54, 58, 60, 66, 70, 76 };  // 217153 + 510510 * n

	static constexpr uint32_t bit4Mask = 1UL << 4;
	static constexpr uint32_t bit5Mask = 1UL << 5;
	static constexpr uint32_t bit6Mask = 1UL << 6;
	static constexpr uint32_t bit7Mask = 1UL << 7;
	static constexpr uint32_t bit8Mask = 1UL << 8;
	static constexpr uint32_t bit9Mask = 1UL << 9;
	static constexpr uint32_t bit10Mask = 1UL << 10;

#define PRIMELIMIT 16384 * 48
#define PRIMELIMIT1 16384 * 14
#define SIEVETARGET 14
#define SIEVETARGETSTART 0

#define MAXCHAIN 5
#define STARTOFFSET 0
#define MAX_PRIMES	8000
#define STOPRANK 3


	CPrimeTest::CPrimeTest()
	{
		const char zTmpPrimeHex[] = "b6122b34ba26087abc80aaa2f75c48d772ec4f8e377ced162efc9a56c167f42ddec5ddcac936f3a0e4ae928b8f61ce451221bd6e71291c0717a667a1418a6bfdb5b1aba05b4d3d5a170e50e05a0d11c4d40075d5cc84625c0bd378f361ed8c438c47b2731dd93f7dfa26ca0f582fca850dafe98bd5c64e8c127462b202ac1bb7";
			
		mpz_init_set_str(zN, zTmpPrimeHex, 16);
		mpz_init_set(zTwo, zN);
		mpz_set_ui(zTwo, 2);			
		mpz_init(zNm1);
		mpz_sub_ui(zNm1, zN, 1L);
		mpz_init(zR);
		//mpz_pow_ui(zR, zN, 2); /* mpz_powm_ui needs excessive memory so preallocate 2x more!!! */
		mpz_realloc(zR, zN->_mp_size * 2);
		
#if defined(_MSC_VER) || defined(__CYGWIN__)
		use_avx2 = 0;
#else

		unsigned long eax, ebx, ecx, edx;
		freebl_cpuid(1, &eax, &ebx, &ecx, &edx);
		has_avx = (ecx & (1 << 28)) != 0 ? 1 : -1;
		freebl_cpuid(7, &eax, &ebx, &ecx, &edx);
		has_avx2 = (ebx & (1 << 5)) != 0 ? 1 : -1;
		use_avx2 = (has_avx > 0) && (has_avx2 > 0);
#endif
	}

	bool CPrimeTest::FermatTest(bool useTrialDivision)
	{
		if (useTrialDivision)
		{
			mp_limb_t r = mpn_preinv_mod_1(zN->_mp_d, (mp_size_t)zN->_mp_size, (mp_limb_t)PP, (mp_limb_t)PP_INVERTED);
			if (r % 3 == 0 || r % 5 == 0 || r % 7 == 0 || r % 11 == 0 || r % 13 == 0 || r % 17 == 0 || r % 19 == 0 || r % 23 == 0 || r % 29 == 0 || r % 31 == 0 || r % 37 == 0 || r % 41 == 0 || r % 43 == 0 || r % 47 == 0 || r % 53 == 0)
				return false;
		}

		zNm1->_mp_d[0] = zN->_mp_d[0] - 1;
		zNm1->_mp_d[1] = zN->_mp_d[1];
		if (use_avx2)
			mp_exptmod(zTwo, zNm1, zN, zR);
		else
			mpz_powm(zR, zTwo, zNm1, zN);	

		if (mpz_cmp_ui(zR, 1L) != 0)
			return false;
		return true;
	}

	int CPrimeTest::FindTuples(unsigned long * candidates, uint16_t * candidateMasks, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, std::vector<std::pair<uint64_t, uint16_t>> * nonces)
	{
		int n1stPrimeSearchLimit = 18;
		int cx = 0;
		int nPrimes = 0;

		mpz_sub(zN, zFirstSieveElement, zPrimeOrigin);
		uint64_t nonce = zN->_mp_d[0];
			
		mpz_set(zN, zFirstSieveElement);
		mpz_set(zNm1, zFirstSieveElement);
		const mp_limb_t d0 = zN->_mp_d[0];
		const mp_limb_t d1 = zN->_mp_d[1];
		int firstPrimeAt;

		uint16_t nStart, nStop, nPrimeCount, nLastOffset;
		while (cx < MAXCANDIDATESPERSIEVE)
		{
			uint64_t candidate = candidates[cx];
			if (candidate == UINT64_MAX)
				break;
			uint16_t candMask = candidateMasks[cx];
			zN->_mp_d[0] = d0;
			zN->_mp_d[1] = d1;
			const uint64_t n = Primorial * candidate;
			cx++;
			mpz_add_ui(zN, zN, n);

			const uint64_t tmp = zN->_mp_d[0];			
			mpz_add_ui(zN, zN, _offsets14Tuple1[5]);
			if (((candMask & bit5Mask) != bit5Mask) && FermatTest())
			{
				n1stPrimeSearchLimit = _offsets14Tuple1[5];
			}
			else
			{
				mpz_add_ui(zN, zN, 2); 
				if (((candMask & bit6Mask) != bit6Mask) && FermatTest())
				{
					candidateHit2Count++;
					n1stPrimeSearchLimit = _offsets14Tuple1[6];
				}
				else
					//{ 					
					//	mpz_add_ui(zN, zN, 6);
					//	if (((candMask & bit7Mask) != bit7Mask) && FermatTest())
					//		n1stPrimeSearchLimit = _offsets14Tuple1[7];
					//	else
					//		continue;
					//}
					continue;				
			}
			nPrimes++;
			candidateHitCount++;
			
			
			zN->_mp_d[0] = tmp;
			//zN->_mp_d[1] = d1;
			nStop = 0; nPrimeCount = 0; nLastOffset = 0; firstPrimeAt = -1;
			double diff = 0;
			bool bTrialDiv = false;
			bool inTupple = false;
			for (nStart = 0; nStart <= nStop + 12; nStart += 2)
			{
				bool bIngnore = false;
				for (int i = 0; i < SIEVETARGET; i++)
				{
					if (_offsets14Tuple1[i] == nLastOffset)
					{
						bTrialDiv = true;
						uint16_t mask = 1U << i;
						if ((candMask & mask) == mask)
							bIngnore = true;
						break;
					}
					else if (_offsets14Tuple1[i] > nLastOffset)
						break;
				}
				if (bIngnore)
				{
					mpz_add_ui(zN, zN, 2);
					nLastOffset += 2;
					continue;
				}

				//if (nStart == 4 || nStart == 10 || nStart == 14 || nStart == 16 || nStart == 22 || nStart == 24 || nStart == 28 || nStart == 34)
				//	bTrialDiv = true;
				//else
				//	bTrialDiv = false;

				//{
				//	mpz_add_ui(zN, zN, 2);
				//	nLastOffset += 2;
				//	continue;
				//}

				if (FermatTest(bTrialDiv))
				//if (mpz_probab_prime_p(zN, 0) > 0) // use this until implement trial division in FermatTest
				{
					nStop = nStart;
					nPrimeCount++;
				}
				if (nPrimeCount == 0 && nStart >= n1stPrimeSearchLimit)
					break;

				if ((firstPrimeAt == -1 && nPrimeCount == 1))
				{
					//mpz_set(zPrimeOriginOffset, zTempVar); // zPrimeOriginOffset = zTempVar
					firstPrimeAt = nStart;
				}

				mpz_add_ui(zN, zN, 2);
				nLastOffset += 2;
				if (nStart >= (nStop + 12) && nLastOffset < n1stPrimeSearchLimit)
				{
					//printf("Stopped at %u before touching search limit %u\n", nLastOffset, n1stPrimeSearchLimit);
					firstPrimeAt = n1stPrimeSearchLimit;
					nLastOffset = n1stPrimeSearchLimit;
					nStop = n1stPrimeSearchLimit;
					nStart = n1stPrimeSearchLimit;
					zN->_mp_d[0] = tmp;
					mpz_add_ui(zN, zN, n1stPrimeSearchLimit);
				}

			}
			if (nPrimeCount >= 3)
			{
				uint64_t nNonce = nonce + n + firstPrimeAt;
				nonces->push_back(std::pair<uint64_t, uint16_t>(nNonce, nPrimeCount));
			}
		}
	candidateCount += cx;
	//if (nonces->size() > 0)
	//	printf("Found %u %u+ tuples out of %u candidates\n", nonces->size(), nMinimumPrimeCount, cx);
	return nPrimes;
	}

	 
	uint32_t * make_primes(unsigned int limit)
	{
		uint32_t      *primes;
		unsigned long       i,j;
		unsigned long       s = sqrtld(prime_limit);
		unsigned long       n = 0;
		bool *bit_array_sieve = (bool*)malloc((prime_limit + 1) * sizeof(bool));
		bit_array_sieve[0] = 0;
		bit_array_sieve[1] = 0;
		for(i=2; i<=prime_limit; i++) bit_array_sieve[i] = 1;
		j = 4;
		while(j<=prime_limit) {
			bit_array_sieve[j] = 0;
			j += 2;
		}
		for(i=3; i<=s; i+=2) {
			if(bit_array_sieve[i] == 1) {
				j = i * 3;
				while(j<=prime_limit) {
					bit_array_sieve[j] = 0;
					j += 2 * i;
				}
			}
		}
		for(i=2;i<=prime_limit;i++) if(bit_array_sieve[i]==1) n += 1;
		primes = (uint32_t*)aligned_alloc(64, (n + 1) * sizeof(uint32_t));
		primes[0] = n;
		j = 1;
		for(i=2;i<=prime_limit;i++) if(bit_array_sieve[i]==1) {
			primes[j] = i;
			j++;
		}
		free(bit_array_sieve);
		return primes;
	}
	
	void InitializePrimes()
	{
		printf("Generating primes...\n");
		// generate prime table

		primes = make_primes(prime_limit);

		printf("%lu primes generated\n", primes[0]);

		mpz_init(zPrimorial);

		mpz_set_ui(zPrimorial, 1);

		for (int i=1; i<nPrimorialEndPrime; i++)
		{
			mpz_mul_ui(zPrimorial, zPrimorial, primes[i]);
		}

		
		printf("\nPrimorial: ");
		mpz_out_str(stdout, 10, zPrimorial); printf("\n");
		

		//printf("\nLast Primorial Prime = %lu\n", primes[nPrimorialEndPrime-1]);

		//printf("\nFirst Sieving Prime = %lu\n", primes[nPrimorialEndPrime]);


		int nSize = mpz_sizeinbase(zPrimorial,2);
		printf("\nPrimorial Size = %d-bit\n\n", nSize);

		inverses=(uint32_t *) aligned_alloc(64, (nPrimeLimit+1)*sizeof(uint32_t));
		memset(inverses, 0, (nPrimeLimit+1) * sizeof(uint32_t));

		mpz_t zPrime, zInverse, zResult;

		mpz_init(zPrime);
		mpz_init(zInverse);
		mpz_init(zResult);

		for(unsigned int i=nPrimorialEndPrime; i<=nPrimeLimit; i++)
		{
			mpz_set_ui(zPrime, primes[i]);

			int	inv = mpz_invert(zResult, zPrimorial, zPrime);
			if (inv <= 0)
			{
				printf("\nNo Inverse for prime %lu at position %u\n\n", primes[i], i);
				exit(0);
			}
			else
			{
				inverses[i]  = mpz_get_ui(zResult);
			}
		}


	}

	unsigned char fastModPrimeChecks(unsigned int n, uint32_t * base_remainders)
	{
		unsigned char rank = MAXCHAIN - STARTOFFSET;
		unsigned char pChainMap[MAXCHAIN];
		for (int j = STARTOFFSET; j < MAXCHAIN; j++)
			pChainMap[j] = 1;

		for (int i = 1; i < MAX_PRIMES; i++)
		{
			int p = primes[i];

			uint64_t idx = base_remainders[i] + (zPrimorial->_mp_d[0] * (uint64_t)n);
			for (int j = STARTOFFSET; j < MAXCHAIN; j++)
			{
				if (pChainMap[j] == 1)
				{
					unsigned int mod = (idx + _offsets14Tuple1[j]) % (uint64_t)p;
					if (mod == 0)
					{
						pChainMap[j] = 0;
						//rank = rank - STARTOFFSET * (j- STARTOFFSET+1);
						rank--;
						if (rank <= STOPRANK)
							return rank;
					}
				}
			}
		}

		return rank;
	}
		
	//struct Cmp
	//{
	bool comparer(const pair<unsigned int, unsigned char> &a, const pair<unsigned int, unsigned char> &b)
	{
		return a.second > b.second;
	}
	//};

//#pragma GCC optimize ("unroll-loops")
	void cpusieve(uint64_t * sieve1, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, uint32_t * primes, uint32_t * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement, unsigned long * candidates)
	{
		mpz_t zPrimorialMod, zTempVar;
		mpz_init(zPrimorialMod);
		mpz_init(zTempVar);

		mpz_mod(zPrimorialMod, zPrimeOrigin, zPrimorial);
		mpz_sub(zPrimorialMod, zPrimorial, zPrimorialMod);

		mpz_mod(zPrimorialMod, zPrimorialMod, zPrimorial);

#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
		mpz_t zKTuplet;
		mpz_init(zKTuplet);
		mpz_import(zKTuplet, 1, 1, sizeof(ktuple_origin), 0, 0, &ktuple_origin);
		mpz_add(zPrimorialMod, zPrimorialMod, zKTuplet);
#else
		mpz_add_ui(zPrimorialMod, zPrimorialMod, ktuple_origin);
#endif

		mpz_add(zTempVar, zPrimeOrigin, zPrimorialMod);
		mpz_set(*zFirstSieveElement, zTempVar);
		uint32_t * base_remainders = (uint32_t *)malloc(PRIMELIMIT * sizeof(uint32_t));

		for (unsigned int i = 0; i < PRIMELIMIT; i++)
		{
			unsigned long p = primes[i];
			base_remainders[i] = mpz_tdiv_ui(zTempVar, p);
		}

		int cx = 0;

		//unsigned int indexes[SIEVETARGET][PRIMELIMIT1];

		// clear the bits
		memset((wchar_t *)sieve1, 0x0, (sieveSize) / 8);

		//for (int pt = SIEVETARGETSTART; pt < SIEVETARGET; pt++)
		//{
		//	for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT1; i++)
		//	{
		//		unsigned long p = primes[i];
		//		unsigned int inv = inverses[i];
		//		unsigned int base_remainder = base_remainders[i];

		//		unsigned int remainder = base_remainder + _offsets14Tuple1[pt];
		//		if (p < remainder)
		//			remainder -= p;
		//		unsigned long r = (p - remainder)*inv;
		//		unsigned int idx = r % p;
		//		indexes[pt][i] = idx;
		//	}
		//}


		for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT1; i++)
		{
			uint32_t  p = primes[i];
			uint32_t inv = inverses[i];
			uint32_t base_remainder = base_remainders[i];

			
			uint32_t lc = (sieveSize / p) + 1;
			for (int l = 0; l < lc; l++)
			{
				uint32_t lp = l*p;
				for (uint32_t pt = SIEVETARGETSTART; pt < SIEVETARGET; pt++)
				{
					//unsigned int idx = indexes[pt][i] + lp;
					uint32_t remainder = base_remainder + _offsets14Tuple1[pt];
					if (p < remainder)
						remainder -= p;
					uint64_t r = (uint64_t)(p - remainder)*(uint64_t)inv;
					uint32_t idx = (r % p) + lp;

					if (idx < sieveSize)
					{
						sieve1[(idx) >> 6] |= (1ULL << ((idx) & 63));
					}
				}
			}


		

		}

		//Sieve extension

		for (unsigned int i = PRIMELIMIT1; i < PRIMELIMIT; i++)
		{
			unsigned long  p = primes[i];
			unsigned long inv = inverses[i];
			unsigned int base_remainder = base_remainders[i];

			uint32_t lc = (sieveSize / primes[i]) + 1;
			uint32_t remainder = base_remainder + _offsets14Tuple1[6];

			if (p < remainder)
				remainder -= p;
			uint64_t r = (uint64_t)(p - remainder)*(uint64_t)inv;
			uint32_t idx = r % p;

			for (uint32_t l = 0; l < lc; l++)
			{
				uint32_t lp = l*p;
				uint32_t idxlp = idx + lp;
				if (idxlp < sieveSize)
					sieve1[(idxlp) >> 6] |= (1ULL << ((idxlp) & 63));
			}

			remainder = base_remainder + _offsets14Tuple1[7];
			if (p < remainder)
				remainder -= p;
			r = (uint64_t)(p - remainder)*(uint64_t)inv;
			idx = r % p;

			for (uint32_t l = 0; l < lc; l++)
			{
				uint32_t lp = l*p;
				uint32_t idxlp = idx + lp;
				if (idxlp < sieveSize)
					sieve1[(idxlp) >> 6] |= (1ULL << ((idxlp) & 63));
			}
		}

		std::list<std::pair<uint64_t, unsigned char>> mapCand;

		for (uint64_t i = 0U; i < sieveSize / 64U; i++)
		{
			if (sieve1[i] == 0xFFFFFFFFFFFFFFFF)
				continue;
			for (uint64_t p = 0U; p < 64U; p++)
			{
				if (sieve1[i] & (1ULL << p))
					continue;
				uint64_t idx = i * 64U + p;
				unsigned char rank = fastModPrimeChecks(idx, base_remainders);
				mapCand.push_back(std::pair<uint64_t, unsigned char>(idx, rank));
			}
		}
		sieveCandidateCount += mapCand.size();
		mapCand.sort(comparer);
		for (auto it = mapCand.begin(); it != mapCand.end(); ) 
		{			
			candidates[cx] = it->first;
			cx++;
			if (cx >= 200)
				break;
			++it;
		}

		candidates[cx] = UINT64_MAX;
		free(base_remainders);
	}

	inline unsigned int GetBitDiff(uint16_t mask, unsigned int SieveTarget)
	{
		int diff = 0;
		int lastPos = -2;
		for (int pt = 0; pt < SieveTarget; pt++)
		{
			uint16_t tMask = (1U << pt);
			uint16_t o = _offsets14Tuple1[pt];
			if ((mask & tMask) != tMask)
			{
				lastPos = (int)o;
				diff++;
			}
			if ((o - lastPos) > 12)
				diff = 0;			
		}
		return diff;
	}

	void AdvancedSieve(uint16_t * sieve1, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, uint32_t * primes, uint32_t * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement, unsigned long * candidates, uint16_t * candidateMasks)
	{
		mpz_t zPrimorialMod, zTempVar;
		mpz_init(zPrimorialMod);
		mpz_init(zTempVar);		
		mpz_mod(zPrimorialMod, zPrimeOrigin, zPrimorial);
		mpz_sub(zPrimorialMod, zPrimorial, zPrimorialMod);
		mpz_mod(zPrimorialMod, zPrimorialMod, zPrimorial);
		//uint64_t offset = 7908189600581ULL;
		//uint64_t offset = 21817283854511261ULL;  // 14-t		
		//uint64_t offset = 841262446570150721ULL; // 14-t
		uint64_t offset = 1006587882969594041ULL; // 14-t
		//uint64_t offset = 1158722981124148367ULL; // k=15  s=56  B={0  2  6  12  14  20  26  30  32  36  42  44  50  54  56} 
//		uint64_t offset = 15131;	// 14-tuple : 15131 + 30030 * n
//		uint64_t offset = 293281;	// 19-tuple : 293281 + 510510 * n
//		uint64_t offset = 217153;	// 19-tuple : 217153 + 510510 * n
//		uint64_t maxu64 = 18446744073709551615ULL;

#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
		mpz_t zKTuplet;	mpz_init(zKTuplet);
		mpz_import(zKTuplet, 1, 1, sizeof(offset), 0, 0, &offset);
		mpz_add(zPrimorialMod, zPrimorialMod, zKTuplet);
#else
		mpz_add_ui(zPrimorialMod, zPrimorialMod, offset);
#endif
		uint32_t maxSieveSegments = 1;
		uint32_t sieveSegmentSize = sieveSize / maxSieveSegments;


		mpz_add(zTempVar, zPrimeOrigin, zPrimorialMod);
		int64_t sieveSegmentOffset = zPrimorial->_mp_d[0] * sieveSegmentSize * ktuple_origin;
		mpz_add_ui(zTempVar, zTempVar, sieveSegmentOffset);

		mpz_set(*zFirstSieveElement, zTempVar);
		
		int cx = 0;

		//for (uint32_t ssIdx = 0; ssIdx < maxSieveSegments; ssIdx++)
		{
			memset(sieve1, 0x0, sieveSegmentSize * sizeof(uint16_t));

			for (unsigned int i = nPrimorialEndPrime; i < nPrimeLimit; i++)
			{
				
				uint32_t  p = primes[i];
				uint32_t inv = inverses[i];				
				uint32_t base_remainder = mpz_tdiv_ui(zTempVar, p);				
				uint32_t lc = (sieveSegmentSize / p) + 1;
				for (int l = 0; l < lc; l++)
				{
					uint32_t lp = l*p;
					for (uint16_t pt = SIEVETARGETSTART; pt < SIEVETARGET; pt++)
					{
						uint32_t remainder = base_remainder + _offsets14Tuple1[pt];
						if (p < remainder)
							remainder -= p;
						uint64_t r = (uint64_t)(p - remainder)*(uint64_t)inv;
						uint32_t idx = (r % p) + lp;

						if (idx < sieveSegmentSize)
							sieve1[idx] |= (1U << pt);
					}
				}
			}

			for (uint64_t i = 0U; i < sieveSegmentSize; i++)
			{
				const uint16_t mask = sieve1[i];
				//if (popcount32(mask & 0xFF00U) < 4)
				if (popcount32(mask) < 6)
				{
					sieveCandidateCount++;
					//if ((mask & 0x20) == 0x20 // bit 5
					//	|| (mask & 0x40) == 0x40 // bit 6 
					//)
					//	continue;

					//if ((mask & 0x80) == 0x80 // bit 7
					//	|| (mask & 0xF0) == 0xF0 // bit 8 
					//)
					//	continue;

					//unsigned int diff =  GetBitDiff(mask, SIEVETARGET);
					//if (diff > 8)
					//if(popcount32(mask & 0x1FFCU) < 4)
					{
						candidates[cx] = i;
						candidateMasks[cx] = mask;
						cx++;
						if (cx >= MAXCANDIDATESPERSIEVE)
							break;
					}
				}
			}
		} // for ssIdx
		candidates[cx] = UINT64_MAX;
	}


	/** Convert Double to unsigned int Representative. Used for encoding / decoding prime difficulty from nBits. **/
	unsigned int SetBits(double nDiff)
	{
		unsigned int nBits = 10000000;
		nBits *= nDiff;
		
		return nBits;
	}

	/** Determines the difficulty of the Given Prime Number.
		Difficulty is represented as so V.X
		V is the whole number, or Cluster Size, X is a proportion
		of Fermat Remainder from last Composite Number [0 - 1] **/
	double GetPrimeDifficulty(CBigNum prime, int checks)
	{
		CBigNum lastPrime = prime;
		CBigNum next = prime + 2;
		unsigned int clusterSize = 1;
		
		///largest prime gap in cluster can be +12
		///this was determined by previously found clusters up to 17 primes
		for( next ; next <= lastPrime + 12; next += 2)
		{
			if(PrimeCheck(next, checks))
			{
				lastPrime = next;
				++clusterSize;
			}
		}
		
		///calulate the rarety of cluster from proportion of fermat remainder of last prime + 2
		///keep fractional remainder in bounds of [0, 1]
		double fractionalRemainder = 1000000.0 / GetFractionalDifficulty(next);
		if(fractionalRemainder > 1.0 || fractionalRemainder < 0.0)
			fractionalRemainder = 0.0;
		
		return (clusterSize + fractionalRemainder);
	}

	double GetSieveDifficulty(CBigNum next, unsigned int clusterSize)
	{
		///calulate the rarety of cluster from proportion of fermat remainder of last prime + 2
		///keep fractional remainder in bounds of [0, 1]
		double fractionalRemainder = 1000000.0 / GetFractionalDifficulty(next);
		if(fractionalRemainder > 1.0 || fractionalRemainder < 0.0)
			fractionalRemainder = 0.0;
		
		return (clusterSize + fractionalRemainder);
	}

	/** Gets the unsigned int representative of a decimal prime difficulty **/
	unsigned int GetPrimeBits(CBigNum prime, int checks)
	{
		return SetBits(GetPrimeDifficulty(prime, checks));
	}

	/** Breaks the remainder of last composite in Prime Cluster into an integer. 
		Larger numbers are more rare to find, so a proportion can be determined 
		to give decimal difficulty between whole number increases. **/
	unsigned int GetFractionalDifficulty(CBigNum composite)
	{
		/** Break the remainder of Fermat test to calculate fractional difficulty [Thanks Sunny] **/
		return ((composite - FermatTest(composite, 2) << 24) / composite).getuint();
	}
	
	/** Determines if given number is Prime. Accuracy can be determined by "checks". 
		The default checks the NXS Network uses is 2 **/
	bool PrimeCheck(CBigNum test, int checks)
	{
		/** Check C: Fermat Tests */
		CBigNum n = 3;
		if(FermatTest(test, n) != 1)
				return false;
		
		return true;
	}

	/** Simple Modular Exponential Equation a^(n - 1) % n == 1 or notated in Modular Arithmetic a^(n - 1) = 1 [mod n]. 
		a = Base or 2... 2 + checks, n is the Prime Test. Used after Miller-Rabin and Divisor tests to verify primality. **/
	CBigNum FermatTest(CBigNum n, CBigNum a)
	{
		CAutoBN_CTX pctx;
		CBigNum e = n - 1;
		CBigNum r;
		BN_mod_exp(&r, &a, &e, &n, pctx);
		
		return r;
	}

	/** Miller-Rabin Primality Test from the OpenSSL BN Library. **/
	bool Miller_Rabin(CBigNum n, int checks)
	{
		return (BN_is_prime(&n, checks, NULL, NULL, NULL) == 1);
	}


	/* Compute T = (P ** -1) mod MP_RADIX.  Also works for 16-bit mp_digits.
	** This technique from the paper "Fast Modular Reciprocals" (unpublished)
	** by Richard Schroeppel (a.k.a. Captain Nemo).
	*/
	mp_digit s_mp_invmod_radix(mp_digit P)
	{
		mp_digit T = P;
		T *= 2 - (P * T);
		T *= 2 - (P * T);
		T *= 2 - (P * T);
		T *= 2 - (P * T);
#if !defined(MP_USE_UINT_DIGIT)
		T *= 2 - (P * T);
		T *= 2 - (P * T);
#endif
		return T;
	}

#define MPZ_DIGIT(MP, N) (MP)->_mp_d[(N)]

	int mp_exptmod(const mpz_ptr inBase, const mpz_ptr exponent, mpz_ptr modulus, mpz_ptr result)
	{
#if defined(_MSC_VER) || defined(__CYGWIN__)
		return MP_UNDEF;
#else
		mp_digit n0;
		mpz_t RR;
		mp_digit a_locl[32] = { 0 };

		mpz_init(RR);
		mpz_set_ui(RR, 1);
		mpz_mul_2exp(RR, RR, modulus->_mp_size * 2 * 64);
		mpz_mod(RR, RR, modulus);
		n0 = 0 - s_mp_invmod_radix(MPZ_DIGIT(modulus, 0));

		memcpy(a_locl, inBase->_mp_d, inBase->_mp_size * 8);

		if (!RSAZ_redundant_mod_exponent((mp_digit*)result->_mp_d, a_locl, (mp_digit*)exponent->_mp_d, (mp_digit *) modulus->_mp_d, (mp_digit*)RR->_mp_d, n0, 1024, 1024, 0))
		{
			return MP_MEM;
		}
		result->_mp_size = modulus->_mp_size;
		while ((result->_mp_d[result->_mp_size - 1] == 0) && result->_mp_size > 0) result->_mp_size--;

		mpz_clear(RR);

		return MP_OKAY;
#endif
	}
}

