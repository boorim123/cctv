/*
 *******************************************************************************
 * Copyright (C) 2011 USOLEX Ltd.
 *
 * Project     : USOLEX DAB System
 * Description : Application Interface Function Implementation
 * Author      : Hyuntai Hwang
 * 
 * Modification History  
 *     2011/05/11 USOLEX Hwnag   first released
 *******************************************************************************
 */

#include "usx_build_options.h"
#include "usx_service_if.h"
#include "usx_typedefs.h"
#include "usx_service.h"
#include "usx_oal.h"
#include "usx_modem_manager.h"
#include "usx_dbgmsg_que.h"
#include "usx_global_variable.h"
#include "usx_memheap.h"

#if (USX_MISRAC == USX_MISRAC_QAC)
#define USX_BUILD_ALLOW 1
#else
#define USX_BUILD_ALLOW 1
#endif

#if (USX_BUILD_ALLOW == 1)

#if ( (USX_OS == USX_OS_LINUX) || (USX_OS == USX_OS_LINUX_X86) )  
#include "cd_audlib_if.h"
#endif

static struct usx_service* s_service = NULL;
struct usx_drvio* g_p_drvio = NULL; 
_i32 g_usx_hal_drv_hdl = -1; 


_usx_ret usx_service_init(void)
{
    _bool bflag;
    _ret ret;

    s_service = NULL;    
    g_p_drvio = NULL;
    g_usx_hal_drv_hdl = -1;

    /* create mem heap */
#if (USX_HEAP_TYPE == USX_HEAP_USOLEX)
    ret = usx_memheap_create();
    if(ret == USX_SUCCESS)
#elif (USX_HEAP_TYPE == USX_HEAP_SYSTEM)    
    if(1)
#else
#error "ERROR : Select Heap Type "
#endif
    {
        /* create oal */
        bflag = usx_oal_is_create();
        if(bflag == USX_FALSE)
        {
            ret = usx_oal_create_obj();
            DEBUG_printf("[usx_service_init] ret = 0x%x\n", ret);
        }
        else
        {
            ret = USX_SUCCESS;
        }
    }

    return ret;
}


_usx_ret usx_service_deinit(void)
{
    _bool bflag;
    _ret ret;
    
    /* destroy usx service */
    if(s_service == NULL)
    {
        ret = USX_FAILURE;
    }
    else
    {
        ret = usx_service_destroy();
        if(ret == USX_SUCCESS)
        {

#if ( (USX_OS != USX_OS_TAMUL) && (USX_OS != USX_OS_NUCLEUS_MV8750))
            ret = usx_dbgmsg_que_destroy_inst();
#endif    

            
        }

    }

/* destroy oal */
    bflag = usx_oal_is_create();
    if(bflag == USX_TRUE)
    {
       if(usx_oal_destroy_obj() != USX_SUCCESS)
       {
       
       }

    }

#if (USX_HEAP_TYPE == USX_HEAP_USOLEX)
    if(usx_memheap_destroy() != USX_SUCCESS)
    {
		/* Nothing to do */
    }

#elif (USX_HEAP_TYPE == USX_HEAP_SYSTEM)    
#else
#error "ERROR : Select Heap Type "
#endif


    return ret;
}


_usx_ui32 usx_service_get_version(void)
{
    return USX_DAB_SUBSYSTEM_VERSION;
}


_usx_ret usx_service_create(struct usx_service_create_config create_config)
{
    _ret ret;
    struct usx_service *p_class;

    if(s_service != NULL)
    {
        ret = USX_ERR_DABSERVICE_ALREADY_OPENED;
    }
    else
    {
        if(usx_oal_is_create() != USX_TRUE)
        {
            USXDEBUGMSG(USX_ZONE_ERROR, (TEXT("[usx_service_create] ERROR: cannot find OAL instance \n\r")));
            ret = USX_ERR_OAL_NOT_CREATED;
        }
        else
        {
            p_class = usx_service_create_obj(create_config);
            if(p_class == NULL)
            {
                ret = USX_ERR_DABSERVICE_SERVICE_CREATE;
            }
            else
            {
                s_service = p_class;
                ret = USX_SUCCESS;
            }
        }
    }

    return ret;
}

_usx_ret usx_service_destroy(void)
{
    _ret ret;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        ret = usx_service_destroy_obj(p_class);
        if(ret == USX_SUCCESS)
        {
            s_service = NULL;
        }
    }

    return ret;
}

_usx_ret usx_service_is_create(void)
{
    return (s_service!=NULL) ? 1U:0U ;
}
 
_usx_ret usx_service_monitor_start(void)
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;

    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->monitor_start!=NULL)
        {
            ret = p_class->monitor_start(p_class);
        }
    }

    return ret;
}
       
_usx_ret usx_service_monitor_stop(void)
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;

    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->monitor_stop!=NULL)
        {
            ret = p_class->monitor_stop(p_class);
        }
    }

    return ret;
}       

_usx_ret usx_service_get_create_config(struct usx_service_create_config* p_create_config)
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;

    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->get_create_config!=NULL)
        {
            ret = p_class->get_create_config(p_class, p_create_config);
        }
    }

    return ret;
}
    
_usx_ret usx_aud_set_swmute_cfg(_usx_ui16 dur, _usx_ui16 dbidx) 
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;

    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->audio_set_sw_mute_config!=NULL)
        {
            ret = p_class->audio_set_sw_mute_config(p_class, dur, dbidx);
        }
    }

    return ret;
}
   
_usx_ret usx_aud_get_swmute_cfg(_usx_ui16 *dur, _usx_ui16 *dbidx) 
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->audio_get_sw_mute_config!=NULL)
        {
            ret = p_class->audio_get_sw_mute_config(p_class, dur, dbidx);
        }
    }

    return ret;
}
   
_usx_ret usx_aud_set_hwmute_status(_usx_ui8 status)  /* 1 : mute  0: unmute */
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->audio_set_hw_mute!=NULL)
        {
            ret = p_class->audio_set_hw_mute(p_class, status);
        }
    }

    return ret;
}
   
_usx_ret usx_aud_get_hwmute_status(_usx_ui8* status)
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->audio_get_hw_mute!=NULL)
        {
            ret = p_class->audio_get_hw_mute(p_class, status);
        }
    }

    return ret;
}

_usx_ret usx_aud_set_drc_status(_usx_ui8  status)
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->audio_set_drc!=NULL)
        {
            ret = p_class->audio_set_drc(p_class, status);
        }
    }

    return ret;
}

_usx_ret usx_aud_get_drc_status(_usx_ui8 *status)
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->audio_get_drc!=NULL)
        {
            ret = p_class->audio_get_drc(p_class, status);
        }
    }

    return ret;
}

_usx_ret usx_aud_set_drc_dbidx(_usx_ui8 dbidx) /* range: 0 ~ 0x3F */
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->audio_set_drc_dbidx!=NULL)
        {
            ret = p_class->audio_set_drc_dbidx(p_class, dbidx);
        }
    }

    return ret;
}
    
_usx_ret usx_aud_get_drc_dbidx(_usx_ui8 *dbidx)/* range: 0 ~ 0x3F */  
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->audio_get_drc_dbidx!=NULL)
        {
            ret = p_class->audio_get_drc_dbidx(p_class, dbidx);
        }
    }

    return ret;
}

_usx_ret usx_aud_set_ec_cfg(struct usx_aud_ec_config cfg)
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->audio_set_ec_cfg!=NULL)
        {
            ret = p_class->audio_set_ec_cfg(p_class, cfg);
        }
    }

    return ret;
}

_usx_ret usx_aud_get_ec_cfg(struct usx_aud_ec_config *p_cfg)
{
    _usx_ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->audio_get_ec_cfg!=NULL)
        {
            ret = p_class->audio_get_ec_cfg(p_class, p_cfg);
        }
    }

    return ret;
}


/************************************************************************
  Player API 
 ***********************************************************************/
_usx_ret usx_player_create(const struct usx_player_create_config* create_config, usx_player_hdl* player_hdl)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_create != NULL)
        {
            ret = p_class->player_create(p_class, create_config, player_hdl);
        }
    }

    return ret;
}
   
_usx_ret usx_player_destroy(usx_player_hdl player_hdl)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_destroy != NULL)
        {
            ret = p_class->player_destroy(p_class, player_hdl);
        }
    }

    return ret;
}

_usx_ret usx_player_play(usx_player_hdl player_hdl)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_play != NULL)
        {
            ret = p_class->player_play(p_class, player_hdl);
        }
    }

    return ret;
}

_usx_ret usx_player_stop(usx_player_hdl player_hdl)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_stop != NULL)
        {
            ret = p_class->player_stop(p_class, player_hdl);
        }
    }

    return ret;
}

_usx_ret usx_player_pause(usx_player_hdl player_hdl)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_pause!=NULL)
        {
            ret = p_class->player_pause(p_class, player_hdl);
        }
    }

    return ret;
}

_usx_ret usx_player_resume(usx_player_hdl player_hdl)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_resume != NULL)
        {
            ret = p_class->player_resume(p_class, player_hdl);
        }
    }

    return ret;
}

/*
_usx_ret usx_player_is_play(usx_player_hdl player_hdl, unsigned int* status)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_is_play != NULL)
        {
            ret = p_class->player_is_play(p_class, player_hdl, status);
        }
    }

    return ret;
}
*/

_usx_ret usx_player_get_cur_channel_info(usx_player_hdl player_hdl, 
                                            struct usx_dab_ensemble_info* p_dab_ensemble_info, 
                                            struct usx_dab_channel_info* p_dab_channel_info 
                                           )
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_get_cur_channel_info != NULL)
        {
            ret = p_class->player_get_cur_channel_info(p_class, player_hdl, p_dab_ensemble_info, p_dab_channel_info);
        }
    }

    return ret;
}

_usx_ret usx_player_get_audio_status(usx_player_hdl player_hdl, struct usx_player_audio_status* p_audio_status)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_get_audio_status!=NULL)
        {
            ret = p_class->player_get_audio_status(p_class, player_hdl, p_audio_status);
        }
    }

    return ret;
}

_usx_ret usx_player_get_audio_quality(usx_player_hdl player_hdl, _usx_ui8* audio_quality, _usx_ui8* p_l)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_get_audio_quality!=NULL)
        {
            ret = p_class->player_get_audio_quality(p_class, player_hdl, audio_quality, p_l);
        }
    }

    return ret;
}

_usx_ret usx_player_get_media_config(usx_player_hdl player_hdl, struct usx_media_config* pmc)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_get_media_config!=NULL)
        {
            ret = p_class->player_get_media_config(p_class, player_hdl, pmc);
        }
    }

    return ret;
}

_usx_ret usx_player_get_mot_object_transfer(usx_player_hdl player_hdl, struct usx_mot_object_transfer *p_mot_object_transfer)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_get_mot_object_transfer!=NULL)
        {
            ret = p_class->player_get_mot_object_transfer(p_class, player_hdl, p_mot_object_transfer);
        }
    }

    return ret;
}

_usx_ret usx_player_get_packet_data_transfer(usx_player_hdl player_hdl, struct usx_packet_data_transfer *p_packet_data_transfer)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_get_packet_data_transfer!=NULL)
        {
            ret = p_class->player_get_packet_data_transfer(p_class, player_hdl, p_packet_data_transfer);
        }
    }

    return ret;
}

_usx_ret usx_player_enable_fidc(usx_player_hdl player_hdl, _bool enable, struct usx_fidc_sc_info fidc_sc_info)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_enable_fidc!=NULL)
        {
            ret = p_class->player_enable_fidc(p_class, player_hdl, enable, fidc_sc_info);
        }
    }

    return ret;
}

_usx_i32 usx_player_get_enable_fidc_count(usx_player_hdl player_hdl)
{
    _i32 cnt = 0;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        if(p_class->player_get_enable_fidc_count!=NULL)
        {
            cnt = p_class->player_get_enable_fidc_count(p_class, player_hdl);
        }
    }

    return cnt;
}

_usx_ret usx_player_get_enable_fidc(usx_player_hdl player_hdl, _usx_i32 idx, struct usx_fidc_sc_info* p_fidc_sc_info)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_get_enable_fidc!=NULL)
        {
            ret = p_class->player_get_enable_fidc(p_class, player_hdl, idx, p_fidc_sc_info);
        }
    }

    return ret;
}

_usx_ret usx_player_get_fidc_data(usx_player_hdl player_hdl, struct usx_fidc_data* p_fidc_data)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_get_fidc_data != NULL)
        {
            ret = p_class->player_get_fidc_data(p_class, player_hdl, p_fidc_data);
        }
    }

    return ret;
}

_usx_ui32 usx_player_get_callback_option(usx_player_hdl player_hdl)
{
    _ui32 cbopt = 0;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        if(p_class->player_get_callback_option != NULL)
        {
            cbopt = p_class->player_get_callback_option(p_class, player_hdl);
        }
    }

    return cbopt;
}
    
_usx_ret usx_player_set_callback_option(usx_player_hdl player_hdl, _usx_ui32 callback_options)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_set_callback_option != NULL)
        {
            ret = p_class->player_set_callback_option(p_class, player_hdl, callback_options);
        }
    }

    return ret;
}

_usx_ret usx_player_get_announcement_list(usx_player_hdl player_hdl, struct usx_announcement_list* p_ann_list)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        if(p_class->player_get_announcement_list != NULL)
        {
            ret = p_class->player_get_announcement_list(p_class, player_hdl, p_ann_list);
        }
    }

    return ret;
}
    
_usx_bool usx_player_is_this_announcement_item(usx_player_hdl player_hdl, const struct usx_announcement_list_item* p_ann_item)
{
    _bool bret = USX_FALSE;
    struct usx_service *p_class;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        if(p_class->player_is_this_announcement_item != NULL)
        {
            bret = p_class->player_is_this_announcement_item(p_class, player_hdl, p_ann_item);
        }
    }

    return bret;
}
        
/*
************************************************************************************************************
*
*  Centre Freq Table API
*
************************************************************************************************************
*/
_usx_ret usx_dab_centre_freq_table_register(struct usx_dab_centre_freq *dab_centre_freq_table, _usx_ui32 freq_count)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            if(p_modem_manager->centre_freq_table_register != NULL)
            {
                ret = p_modem_manager->centre_freq_table_register(p_modem_manager, dab_centre_freq_table, freq_count);
            }
        }
    }

    return ret;
}

/*
_usx_ret usx_dab_centre_freq_table_add(struct usx_dab_centre_freq *p_dab_centre_freq)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        return USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {

        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->centre_freq_table_add(p_modem_manager, p_dab_centre_freq);
        }
    }

    return ret;
}
*/

/*
_usx_ret usx_dab_centre_freq_table_delete(struct usx_dab_centre_freq *p_dab_centre_freq)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->centre_freq_table_delete(p_modem_manager, p_dab_centre_freq);
        }
    }

    return ret;
}
*/

/*
_usx_i32 usx_dab_centre_freq_table_get_freq_count(void)
{
    _i32 iret = 0;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        iret = 0;
    }
    else
    {

        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            iret = 0;
        }
        else
        {
            iret = p_modem_manager->centre_freq_table_get_freq_count(p_modem_manager);
        }
    }

    return iret;
}
*/

/*
_usx_ret usx_dab_centre_freq_table_get_freq(_i32 index, struct usx_dab_centre_freq * p_dab_centre_freq)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->centre_freq_table_get_freq(p_modem_manager, index, p_dab_centre_freq);
        }
    }

    return ret;
}
*/

/*
_usx_ret usx_dab_centre_freq_table_get_cur_freq(struct usx_dab_centre_freq * p_dab_centre_freq, _i32* index)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->centre_freq_table_get_cur_freq(p_modem_manager, p_dab_centre_freq, index);
        }
    }

    return ret;
}
*/

/*
_usx_ret usx_dab_centre_freq_table_get_prev_freq(struct usx_dab_centre_freq * p_dab_centre_freq, _i32* index)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->centre_freq_table_get_prev_freq(p_modem_manager, p_dab_centre_freq, index);
        }
    }

    return ret;
}
*/

/*
_usx_ret usx_dab_centre_freq_table_get_next_freq(struct usx_dab_centre_freq * p_dab_centre_freq, _i32* index)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        return USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->centre_freq_table_get_next_freq(p_modem_manager, p_dab_centre_freq, index);
        }
    }

    return ret;
}
*/

/*
************************************************************************************************************
*
*  Tune API
*
************************************************************************************************************
*/
_usx_ret usx_dab_tune_start(const struct usx_dab_tune_config *p_dab_tune_config)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_start(p_modem_manager, p_dab_tune_config);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_stop(void)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_stop(p_modem_manager);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_change_freq(_usx_ui32 freq)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            if(p_modem_manager->tune_change_freq != NULL)
            {
                ret = p_modem_manager->tune_change_freq(p_modem_manager, freq);
            }
            else
            {
                ret = USX_FAILURE;
            }
        }
    }

    return ret;
}

_usx_bool usx_dab_tune_is_start(void)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_is_start(p_modem_manager);
        }
    }

    return ret;
}

_usx_bool usx_dab_tune_is_done(void)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret =p_modem_manager->tune_is_done(p_modem_manager);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_init_tuned_tree(void)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager *p_modem_manager;
    struct usx_dab_database *p_database;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            p_database = p_modem_manager->get_dab_database(p_modem_manager);
            if(p_database != NULL)
                ret = p_database->init_tuned_tree(p_database);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_set_ens_info_of_tuned_tree(struct usx_dab_ensemble_info *p_dab_ensemble_info)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    struct usx_dab_database *p_database;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            p_database = p_modem_manager->get_dab_database(p_modem_manager);
            if(p_database != NULL)
                ret = p_database->set_tuned_ensemble_info(p_database, p_dab_ensemble_info);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_set_srv_info_of_tuned_tree(_usx_i32 idx, struct usx_dab_service_info *p_dab_service_info)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    struct usx_dab_database *p_database;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            p_database = p_modem_manager->get_dab_database(p_modem_manager);
            if(p_database != NULL)
                ret = p_database->set_tuned_service_info(p_database, idx, p_dab_service_info);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_set_ch_info_of_tuned_tree(_usx_i32 idx, struct usx_dab_channel_info *p_dab_channel_info)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    struct usx_dab_database *p_database;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            p_database = p_modem_manager->get_dab_database(p_modem_manager);
            if(p_database != NULL)
                ret = p_database->set_tuned_channel_info(p_database, idx, p_dab_channel_info);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_get_channel_info_radio(struct usx_dab_ensemble_info *p_dab_ensemble_info, struct usx_dab_channel_info *p_dab_channel_info)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_get_channel_info_radio(p_modem_manager, p_dab_ensemble_info, p_dab_channel_info);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_get_channel_info_epg(struct usx_dab_ensemble_info *p_dab_ensemble_info, struct usx_dab_channel_info *p_dab_channel_info)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret =p_modem_manager->tune_get_channel_info_epg(p_modem_manager, p_dab_ensemble_info, p_dab_channel_info);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_get_freq(_usx_ui32 *freq)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_get_freq(p_modem_manager, freq);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_get_ensemble_info(struct usx_dab_ensemble_info *p_dab_ensemble_info)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_get_ensemble_info(p_modem_manager, p_dab_ensemble_info);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_get_service_info(_usx_i32 srvidx, _usx_ui8 tree_version, struct usx_dab_service_info *p_dab_service_info)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_get_service_info(p_modem_manager, srvidx, tree_version, p_dab_service_info);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_get_channel_info(_usx_i32 chidx, _usx_ui8 tree_version, struct usx_dab_channel_info *p_dab_channel_info)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_get_channel_info(p_modem_manager, chidx, tree_version, p_dab_channel_info);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_get_service_info_with_sid(_usx_ui32 sid, struct usx_dab_service_info *p_dab_service_info)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_get_service_info_with_sid(p_modem_manager, sid, p_dab_service_info);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_get_ann_channel_info(_usx_ui8 anntype, _usx_ui8 clustid, _usx_ui8 subchid, struct usx_dab_channel_info_id *dab_channel_info_id)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    struct usx_dab_database* p_dab_database;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_FAILURE;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_FAILURE;
        }
        else
        {

            p_dab_database = p_modem_manager->get_dab_database(p_modem_manager);
            if(p_dab_database!=NULL)
            {
                if(p_dab_database->get_ann_channel_info!=NULL)
                {
                    ret = p_dab_database->get_ann_channel_info(p_dab_database, anntype, clustid, subchid, dab_channel_info_id);
                }
            }
        }

    }

    return ret;
}

_usx_bool usx_dab_tune_is_tuned(void)
{
    _bool bret = USX_FALSE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
        
        }
        else
        {
            bret = p_modem_manager->tune_is_tuned(p_modem_manager);
        }
    }

    return bret;
}

_usx_ui32 usx_dab_tune_get_ensemble_parm(_usx_ui32 parm_type)
{
    _ui32 iret = 0;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
         
        }
        else
        {
            iret = p_modem_manager->tune_get_ensemble_parm(p_modem_manager, parm_type);
        }
    }

    return iret;
}
    
_usx_ui32 usx_dab_tune_get_service_parm(_usx_i32 chidx, _usx_ui8 tree_version, _usx_ui32 parm_type)
{
    _ui32 iret = 0;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            iret = p_modem_manager->tune_get_service_parm(p_modem_manager, chidx, tree_version, parm_type);
        }
    }

    return iret;
}

_usx_ui32 usx_dab_tune_get_service_parm_with_sid(_usx_ui32 sid, _usx_ui32 parm_type)
{
    _ui32 iret = 0;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            iret = p_modem_manager->tune_get_service_parm_with_sid(p_modem_manager, sid, parm_type);
        }
    }

    return iret;
}

_usx_ret usx_dab_tune_get_service_label_with_sid(_usx_ui32 sid, struct usx_dab_info_label* p_dab_info_label)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            ret = p_modem_manager->tune_get_service_label_with_sid(p_modem_manager, sid, p_dab_info_label);
        }
    }

    return ret;
}

_usx_ui32 usx_dab_tune_get_channel_parm_with_sid_cid(_usx_ui32 sid, _usx_ui16 cid, _usx_ui32 parm_type)
{
    _ui32 iret = 0;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            iret = p_modem_manager->tune_get_channel_parm_with_sid_cid(p_modem_manager, sid, cid, parm_type);
        }
    }

    return iret;
}

_usx_ret usx_dab_tune_get_channel_label_with_sid_cid(_usx_ui32 sid, _usx_ui16 cid, struct usx_dab_info_label* p_dab_info_label)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            ret = p_modem_manager->tune_get_channel_label_with_sid_cid(p_modem_manager, sid, cid, p_dab_info_label);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_get_channel_cid(_usx_ui32 sid, _usx_ui8 scids, _usx_ui16* cid)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            ret = p_modem_manager->tune_get_channel_cid(p_modem_manager, sid, scids, cid);
        }
    }

    return ret;
}

_usx_ui32 usx_dab_tune_get_channel_parm(_usx_i32 chidx, _usx_ui8 tree_version, _usx_ui32 parm_type)
{
    _ui32 iret = 0;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            iret = p_modem_manager->tune_get_channel_parm(p_modem_manager, chidx, tree_version, parm_type);
        }
    }

    return iret;
}
   
_usx_ret usx_dab_tune_get_ensemble_label(struct usx_dab_info_label* p_dab_info_label)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            ret = p_modem_manager->tune_get_ensemble_label(p_modem_manager, p_dab_info_label);
        }
    }

    return ret;
}
    
_usx_ret usx_dab_tune_get_service_label(_usx_i32 chidx, _usx_ui8 tree_version, struct usx_dab_info_label* p_dab_info_label)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            ret = p_modem_manager->tune_get_service_label(p_modem_manager, chidx, tree_version, p_dab_info_label);
        }
    }

    return ret;
}
    
_usx_ret usx_dab_tune_get_channel_label(_usx_i32 chidx, _usx_ui8 tree_version, struct usx_dab_info_label* p_dab_info_label)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            ret = p_modem_manager->tune_get_channel_label(p_modem_manager, chidx, tree_version, p_dab_info_label);
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_db_reset(void)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            ret = p_modem_manager->tune_db_reset(p_modem_manager);
        }
    }

    return ret;
}

_usx_bool usx_dab_tune_is_this_radio_channel(_usx_ui32 sid, _usx_ui16 cid)
{
    _bool bret = USX_FALSE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    struct usx_dab_database* p_dab_database;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            p_dab_database = p_modem_manager->get_dab_database(p_modem_manager);
            if(p_dab_database!=NULL)
            {
                if(p_dab_database->is_this_radio_channel!=NULL)
                {
                    bret = p_dab_database->is_this_radio_channel(p_dab_database, sid, cid);
                }
            }
        }
    }

    return bret;
}

_usx_ui8  usx_dab_tune_get_channel_tmid(_usx_ui32 sid, _usx_ui16 cid)
{
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    struct usx_dab_database* p_dab_database;
    _ui8 tmid1 = 0xFF;

    p_class = s_service;
    if(p_class == NULL)
    {
        
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
        
        }
        else
        {
            p_dab_database = p_modem_manager->get_dab_database(p_modem_manager);
            if(p_dab_database!=NULL)
            {
                if(p_dab_database->get_channel_tmid!=NULL)
                {
                    tmid1 = p_dab_database->get_channel_tmid(p_dab_database, sid, cid);
                }
            }
        }
    }

    return tmid1;
}
   
_usx_ret usx_dab_tune_get_epg_sid_cid(_usx_ui32* p_sid_ret, _usx_ui16* p_cid_ret)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    struct usx_dab_database* p_dab_database;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            p_dab_database = p_modem_manager->get_dab_database(p_modem_manager);
            if(p_dab_database!=NULL)
            {
                ret = p_dab_database->get_epg_sid_cid(p_dab_database, p_sid_ret, p_cid_ret );
            }
        }
    }

    return ret;
}

_usx_ret usx_dab_tune_register_eti_file(_usx_ui32 freq, const _usx_ui8* eti_file_name)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    
    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_register_eti_file(p_modem_manager, freq, eti_file_name);
        }
    }

    return ret;
}
   
_usx_ret usx_dab_tune_unregister_eti_file(_usx_ui32 freq)
{
    _ret ret;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    
    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            ret = p_modem_manager->tune_unregister_eti_file(p_modem_manager, freq);
        }
    }

    return ret;
}

/* user app info */
_usx_ret usx_dab_tune_get_fig13(_usx_ui32 sid, _usx_ui8 scids, struct usx_fic_info_user_app_info* p_fic_info_user_app_info) 
{
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    struct usx_dab_database* p_dab_database;
    _ret ret = USX_FAILURE; 

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }   
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            p_dab_database = p_modem_manager->get_dab_database(p_modem_manager);
            if(p_dab_database!=NULL)
            {
                if(p_dab_database->get_fig13!=NULL)
                {
                    ret = p_dab_database->get_fig13(p_dab_database, sid, scids, p_fic_info_user_app_info);
                }
            }
        }
    }

    return ret;
}

_usx_ui32 usx_dab_tune_get_visteon_sid(_usx_ui32 sid) /* convert visteon sid*/
{
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;
    struct usx_dab_database* p_dab_database;
    _ui32 visteon_sid = 0xFFFFFFFFU;

    p_class = s_service;
    if(p_class == NULL)
    {
    
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            
        }
        else
        {
            p_dab_database = p_modem_manager->get_dab_database(p_modem_manager);
            if(p_dab_database!=NULL)
            {
                if(p_dab_database->get_visteon_sid!=NULL)
                {
                    visteon_sid = p_dab_database->get_visteon_sid(p_dab_database, sid);
                }
            }
        }
    }

    return visteon_sid;
}

/**************************************************************** 
   demod api 
****************************************************************/

_usx_ret usx_demod_set_tune_param(_usx_ui32 ofdm_lock_timeout, _usx_ui32 fic_dec_timeout, _usx_ui32 ofdm_signal_timeout)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            if(p_modem_manager->demod_set_tune_param != NULL)
            {
                ret = p_modem_manager->demod_set_tune_param(p_modem_manager, ofdm_lock_timeout, fic_dec_timeout, ofdm_signal_timeout);
            }
        }
    }

    return ret;
}

_usx_ret usx_demod_get_tune_param(_usx_ui32 *ofdm_lock_timeout, _usx_ui32 *fic_dec_timeout, _usx_ui32 *ofdm_signal_timeout)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            if(p_modem_manager->demod_get_tune_param != NULL)
            {
                ret = p_modem_manager->demod_get_tune_param(p_modem_manager, ofdm_lock_timeout, fic_dec_timeout, ofdm_signal_timeout);
            }
        }
    }

    return ret;
}

_usx_ret usx_demod_get_rf_info(struct usx_rf_info *p_rf_info)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            if(p_modem_manager->demod_get_rf_info != NULL)
            {
                ret = p_modem_manager->demod_get_rf_info(p_modem_manager, p_rf_info);
            }
        }
    }

    return ret;
}

_usx_ret usx_demod_enable_zerober(_usx_bool enable)
{
    _ret ret = USX_FAILURE;
    struct usx_service *p_class;
    struct usx_modem_manager* p_modem_manager;

    p_class = s_service;
    if(p_class == NULL)
    {
        ret = USX_ERR_DABSERVICE_NOT_OPENED;
    }
    else
    {
        p_modem_manager = p_class->modem_get_manager(p_class);
        if(p_modem_manager == NULL)
        {
            ret = USX_ERR_MODEM_NOT_OPENED;
        }
        else
        {
            if(p_modem_manager->demod_enable_zerober != NULL)
            {
                ret = p_modem_manager->demod_enable_zerober(p_modem_manager, enable);
            }
        }
    }

    return ret;
}

#endif /* #if (USX_BUILD_ALLOW == 1) */
