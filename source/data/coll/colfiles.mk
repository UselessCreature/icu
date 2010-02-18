# *   Copyright (C) 1998-2010, International Business Machines
# *   Corporation and others.  All Rights Reserved.
COLLATION_CLDR_VERSION = 1.7
# A list of txt's to build
# Note:
#
#   If you are thinking of modifying this file, READ THIS.
#
# Instead of changing this file [unless you want to check it back in],
# you should consider creating a 'collocal.mk' file in this same directory.
# Then, you can have your local changes remain even if you upgrade or
# reconfigure ICU.
#
# Example 'collocal.mk' files:
#
#  * To add an additional locale to the list:
#    _____________________________________________________
#    |  COLLATION_SOURCE_LOCAL =   myLocale.txt ...
#
#  * To REPLACE the default list and only build with a few
#    locales:
#    _____________________________________________________
#    |  COLLATION_SOURCE = ar.txt ar_AE.txt en.txt de.txt zh.txt
#
#
# Generated by LDML2ICUConverter, from LDML source files.

# Aliases without a corresponding xx.xml file (see icu-config.xml & build.xml)
COLLATION_SYNTHETIC_ALIAS = de_.txt de__PHONEBOOK.txt es_.txt es__TRADITIONAL.txt\
 hi_.txt hi__DIRECT.txt in.txt in_ID.txt iw.txt\
 iw_IL.txt no.txt no_NO.txt pa_IN.txt sh.txt\
 sh_BA.txt sh_YU.txt sr_BA.txt sr_ME.txt sr_RS.txt\
 zh_.txt zh_CN.txt zh_HK.txt zh_MO.txt zh_SG.txt\
 zh_TW.txt zh_TW_STROKE.txt zh__PINYIN.txt


# All aliases (to not be included under 'installed'), but not including root.
COLLATION_ALIAS_SOURCE = $(COLLATION_SYNTHETIC_ALIAS)


# Empty locales, used for validSubLocale fallback.
COLLATION_EMPTY_SOURCE = af_NA.txt af_ZA.txt ar_AE.txt ar_BH.txt\
 ar_DZ.txt ar_EG.txt ar_IQ.txt ar_JO.txt ar_KW.txt\
 ar_LB.txt ar_LY.txt ar_MA.txt ar_OM.txt ar_QA.txt\
 ar_SA.txt ar_SD.txt ar_SY.txt ar_TN.txt ar_YE.txt\
 as_IN.txt az_Latn.txt az_Latn_AZ.txt be_BY.txt bg_BG.txt\
 bn_BD.txt bn_IN.txt ca_ES.txt cs_CZ.txt cy_GB.txt\
 da_DK.txt de_AT.txt de_BE.txt de_CH.txt de_DE.txt\
 de_LU.txt el_GR.txt en_AU.txt en_BW.txt en_CA.txt\
 en_GB.txt en_HK.txt en_IE.txt en_IN.txt en_MT.txt\
 en_NZ.txt en_PH.txt en_SG.txt en_US.txt en_US_POSIX.txt\
 en_VI.txt en_ZA.txt en_ZW.txt es_AR.txt es_BO.txt\
 es_CL.txt es_CO.txt es_CR.txt es_DO.txt es_EC.txt\
 es_ES.txt es_GT.txt es_HN.txt es_MX.txt es_NI.txt\
 es_PA.txt es_PE.txt es_PR.txt es_PY.txt es_SV.txt\
 es_US.txt es_UY.txt es_VE.txt et_EE.txt fa_IR.txt\
 fi_FI.txt fo_FO.txt fr_BE.txt fr_CA.txt fr_CH.txt\
 fr_FR.txt fr_LU.txt ga.txt ga_IE.txt gu_IN.txt\
 ha_Latn.txt ha_Latn_GH.txt ha_Latn_NE.txt ha_Latn_NG.txt he_IL.txt\
 hi_IN.txt hr_HR.txt hu_HU.txt hy_AM.txt id.txt\
 id_ID.txt ig_NG.txt is_IS.txt it_CH.txt it_IT.txt\
 ja_JP.txt ka.txt ka_GE.txt kk_KZ.txt kl_GL.txt\
 kn_IN.txt ko_KR.txt kok_IN.txt lt_LT.txt lv_LV.txt\
 mk_MK.txt ml_IN.txt mr_IN.txt ms.txt ms_BN.txt\
 ms_MY.txt mt_MT.txt nb_NO.txt nl.txt nl_BE.txt\
 nl_NL.txt nn_NO.txt om_ET.txt om_KE.txt or_IN.txt\
 pa_Arab.txt pa_Arab_PK.txt pa_Guru.txt pa_Guru_IN.txt pl_PL.txt\
 ps_AF.txt pt.txt pt_BR.txt pt_PT.txt ro_RO.txt\
 ru_RU.txt ru_UA.txt si_LK.txt sk_SK.txt sl_SI.txt\
 sq_AL.txt sr_Cyrl.txt sr_Cyrl_BA.txt sr_Cyrl_ME.txt sr_Cyrl_RS.txt\
 sr_Latn_BA.txt sr_Latn_ME.txt sr_Latn_RS.txt st.txt st_LS.txt\
 st_ZA.txt sv_FI.txt sv_SE.txt sw_KE.txt sw_TZ.txt\
 ta_IN.txt te_IN.txt th_TH.txt tr_TR.txt uk_UA.txt\
 ur_IN.txt ur_PK.txt vi_VN.txt xh.txt xh_ZA.txt\
 yo_NG.txt zh_Hans.txt zh_Hans_CN.txt zh_Hans_SG.txt zh_Hant_HK.txt\
 zh_Hant_MO.txt zh_Hant_TW.txt zu.txt zu_ZA.txt


# Ordinary resources
COLLATION_SOURCE = $(COLLATION_EMPTY_SOURCE) af.txt ar.txt as.txt az.txt\
 be.txt bg.txt bn.txt ca.txt cs.txt\
 cy.txt da.txt de.txt el.txt en.txt\
 en_BE.txt eo.txt es.txt et.txt fa.txt\
 fa_AF.txt fi.txt fil.txt fo.txt fr.txt\
 gu.txt ha.txt haw.txt he.txt hi.txt\
 hr.txt hu.txt hy.txt ig.txt is.txt\
 it.txt ja.txt kk.txt kl.txt km.txt\
 kn.txt ko.txt kok.txt lt.txt lv.txt\
 mk.txt ml.txt mr.txt mt.txt nb.txt\
 nn.txt om.txt or.txt pa.txt pl.txt\
 ps.txt ro.txt ru.txt si.txt sk.txt\
 sl.txt sq.txt sr.txt sr_Latn.txt sv.txt\
 sw.txt ta.txt te.txt th.txt to.txt\
 tr.txt uk.txt ur.txt vi.txt yo.txt\
 zh.txt zh_Hant.txt

