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
/*
namespace rgw::sal {






    int TUser::list_buckets(const DoutPrefixProvider* dpp, const std::string& marker,
			       const std::string& end_marker, uint64_t max, bool need_stats,
			       BucketList &buckets, optional_yield y)
    {
        return user.list_buckets(dpp, marker, end_marker, max, need_stats, buckets, y);
    }

    int TUser::create_bucket(const DoutPrefixProvider *dpp,
      const rgw_bucket& b,
      const string& zonegroup_id,
      rgw_placement_rule& placement_rule,
      string& swift_ver_location,
      const RGWQuotaInfo * pquota_info,
      const RGWAccessControlPolicy& policy,
      Attrs& attrs,
      RGWBucketInfo& info,
      obj_version& ep_objv,
      bool exclusive,
      bool obj_lock_enabled,
      bool *existed,
      req_info& req_info,
      std::unique_ptr<Bucket>* bucket_out,
      optional_yield y)
    {
        return user.create_bucket(dpp, b, zonegroup_id, bucket, placement_rule,
                            swift_ver_location, &pquota_info, policy, attrs,
                            info, ep_objv, exclusive, obj_lock_enabled, &existed, req_info,
                            &bucket_out, y);
    }

    int TUser::read_attrs(const DoutPrefixProvider* dpp, optional_yield y)
    {
        return user.read_attrs(dpp, y);
    }
    
    int TUser::read_stats(const DoutPrefixProvider *dpp,
        optional_yield y, RGWStorageStats* stats,
        ceph::real_time *last_stats_sync,
        ceph::real_time *last_stats_update)
        {
            return user.read_stats(dpp, y, stats, last_stats_sync, last_stats_update);
        }

    int TUser::read_stats_async(const DoutPrefixProvider *dpp, RGWGetUserStats_CB *cb)
  {
      return user.read_stats_async(dpp, cb);
  }
 //I want to get bootup to run before adding any other functions. Dan P
#ifdef 0
  const RGWZoneGroup& TZone::get_zonegroup()
  {
    return store->get_store()->get_zone()->get_zonegroup();
  }
  
  int TStore::register_to_service_map(const DoutPrefixProvider *dpp, const string& daemon_type,
      const map<string, string>& meta)
  {
    return store->register_to_service_map(dpp, daemon_type, meta);
  }



 } //namespace rgw::sal

 extern "C" {

     void*newTStore(CephContext *cct, const std::string config_store) //may need to also feed in a string for either rados or dbstore. Dan P
    {
        rgw::sal::TStore *store = new rgw::sal::TStore(); //TODO: make sure that the constructor is ready. Dan P
        if (store) {
    #ifdef WITH_RADOSGW_DBSTORE
        if(config_store == "dbstore") {

            //set up a dbstore and assign it to the Tstore
        }
    #endif
        if(config_store == "rados") {

            //set up a radosStore and assign it to the Tstore
        }
        }

        return store;
    }
#endif

 }*/