// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp

/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2021 Red Hat, Inc.
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation. See file COPYING.
 *
 */

 //TODO: fix the indents and tabs - Dan P


//shared includes
#include <errno.h>
#include "common/errno.h"
#include <stdlib.h>
#include <system_error>
#include <unistd.h>
#include <sstream>

#include "common/Clock.h"

#include "rgw_sal.h"
#include "rgw_sal_rados.h"
#include "rgw_sal_dbstore.h"
#include "rgw_bucket.h"

#include "rgw_sal_tracer.h"
//rados includes
/*
#include "rgw_multi.h"
#include "rgw_acl_s3.h"
#include "rgw_aio.h"
#include "rgw_aio_throttle.h"
#include "rgw_tracer.h"

#include "rgw_zone.h"
#include "rgw_rest_conn.h"
#include "rgw_service.h"
#include "rgw_lc.h"
#include "services/svc_sys_obj.h"
#include "services/svc_zone.h"
#include "services/svc_tier_rados.h"
#include "services/svc_quota.h"
#include "services/svc_config_key.h"
#include "services/svc_zone_utils.h"
#include "cls/rgw/cls_rgw_client.h"

#include "rgw_pubsub.h"
*/
//dbstore includes

#define dout_subsys ceph_subsys_rgw

using namespace std;

namespace rgw::sal {

    /*user functions*/
    int TracerUser::remove_user(const DoutPrefixProvider* dpp, optional_yield y)
    {
        return realUser->remove_user(dpp, y); //may need to also remove this user - Dan P
    }
    int TracerUser::load_user(const DoutPrefixProvider *dpp, optional_yield y)
    {
        return realUser->load_user(dpp, y);
    }
    int TracerUser::store_user(const DoutPrefixProvider* dpp, optional_yield y, bool exclusive, RGWUserInfo* old_info)
    {
        return realUser->store_user(dpp, y, exclusive, old_info);
    }

    int TracerUser::create_bucket(const DoutPrefixProvider* dpp,
          const rgw_bucket& b,
          const std::string& zonegroup_id,
          rgw_placement_rule& placement_rule,
          std::string& swift_ver_location,
          const RGWQuotaInfo* pquota_info,
          const RGWAccessControlPolicy& policy,
          Attrs& attrs,
          RGWBucketInfo& info,
          obj_version& ep_objv,
          bool exclusive,
          bool obj_lock_enabled,
          bool* existed,
          req_info& req_info,
          std::unique_ptr<Bucket>* bucket,
          optional_yield y)
    {
        return realUser->create_bucket(dpp, b, zonegroup_id, placement_rule, swift_ver_location, pquota_info, policy,
                attrs, info, ep_objv, exclusive, obj_lock_enabled, existed, req_info, bucket, y);
    }

    int TracerUser::list_buckets(const DoutPrefixProvider* dpp, const std::string& marker,
			       const std::string& end_marker, uint64_t max, bool need_stats,
			       BucketList &buckets, optional_yield y)
    {
        return realUser->list_buckets(dpp, marker, end_marker, max, need_stats, buckets, y);
    }


    int TracerUser::read_attrs(const DoutPrefixProvider* dpp, optional_yield y)
    {
        return realUser->read_attrs(dpp, y);
    }
    
    int TracerUser::read_stats(const DoutPrefixProvider *dpp,
        optional_yield y, RGWStorageStats* stats,
        ceph::real_time *last_stats_sync,
        ceph::real_time *last_stats_update)
    {
            return realUser->read_stats(dpp, y, stats, last_stats_sync, last_stats_update);
    }

    int TracerUser::read_stats_async(const DoutPrefixProvider *dpp, RGWGetUserStats_CB *cb)
    {
      return realUser->read_stats_async(dpp, cb);
    }

    int TracerUser::merge_and_store_attrs(const DoutPrefixProvider* dpp, Attrs& new_attrs, optional_yield y)
    {
        return realUser->merge_and_store_attrs(dpp, new_attrs, y);
    }

    int TracerUser::complete_flush_stats(const DoutPrefixProvider *dpp, optional_yield y)
    {
        return realUser->complete_flush_stats(dpp, y);
    }

    int TracerUser::read_usage(const DoutPrefixProvider *dpp, uint64_t start_epoch, uint64_t end_epoch, uint32_t max_entries,
    bool *is_truncated, RGWUsageIter& usage_iter,
    map<rgw_user_bucket, rgw_usage_log_entry>& usage)
    {
        return realUser->read_usage(dpp, start_epoch, end_epoch, max_entries, is_truncated, usage_iter, usage);
    }

  int TracerUser::trim_usage(const DoutPrefixProvider *dpp, uint64_t start_epoch, uint64_t end_epoch)
  {
    return realUser->trim_usage(dpp, start_epoch, end_epoch);
  }

 /*Zonegroup functions */

  const RGWZoneParams& TZone::get_params()
  {
    return realZone->get_params();
  }

  const rgw_zone_id& TZone::get_id()
  {
    return realZone->get_id();
  }

  const RGWRealm& TZone::get_realm()
  {
    return realZone->get_realm();
  }

  bool TZone::is_writeable()
  {
    return realZone->is_writeable();
  }

  bool TZone::has_zonegroup_api(const std::string& api) const
  {
    return realZone->has_zonegroup_api(api);
  }

  const std::string& TZone::get_current_period_id()
  {
    return realZone->get_current_period_id();
  }

  bool TZone::get_redirect_endpoint(std::string* endpoint)
  {
    return realZone->get_redirect_endpoint(endpoint);
  }

  const std::string& TZone::get_name() const
  {
    return realZone->get_name();
  }

  const RGWZoneGroup& TZone::get_zonegroup()
  {
    return realZone->get_zonegroup();
  }
  
  int TZone::get_zonegroup(const std::string& id, RGWZoneGroup& zg)
  {
      return realZone->get_zonegroup(id, zg);
  }
  /*Tracer Driver functions */

  
  int TracerDriver::register_to_service_map(const DoutPrefixProvider *dpp, const string& daemon_type,
      const map<string, string>& meta)
  {
    return realStore->register_to_service_map(dpp, daemon_type, meta);
  }

  void TracerDriver::finalize(void)
  {
      if(realStore)
      realStore->finalize(); //May need to implement additional cleanup for this store itself.
  }



 } //namespace rgw::sal

 extern "C" {

    void* newTracer(rgw::sal::Store* inputStore) /*takes in a store and wraps */ //may need to also feed in a string for either rados or dbstore. Dan P
    {
        rgw::sal::TracerDriver *trace = new rgw::sal::TracerDriver(); //TODO: make sure that the constructor is ready. Dan P
        trace->initialize(inputStore);
        if (trace) {
            return trace;
        }
        return NULL;
    }

 }