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
 //I want to get bootup to run before adding any other functions. Dan P
  const RGWZoneGroup& TZone::get_zonegroup()
  {
    return trace->get_zone()->get_zonegroup();
  }
  
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