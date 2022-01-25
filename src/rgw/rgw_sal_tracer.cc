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


#define dout_subsys ceph_subsys_rgw

using namespace std;

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

 } //namespace rgw::sal