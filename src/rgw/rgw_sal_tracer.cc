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
#include "common/dout.h"

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
//for dout
#define dout_context g_ceph_context

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
        
        ldpp_dout(dpp, 20) << "TRACER: recieved operation: create_bucket" << dendl;
        int r;
        r = realUser->create_bucket(dpp, b, zonegroup_id, placement_rule, swift_ver_location, pquota_info, policy,
                attrs, info, ep_objv, exclusive, obj_lock_enabled, existed, req_info, bucket, y);
        ldpp_dout(dpp, 20) << "TRACER: Primary store recieved and carried out operation: create bucket" << dendl;
        return r;
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

  std::unique_ptr<User> TracerDriver::get_user(const rgw_user &u)
  {
    dout(20) << "TRACER: intercepted operation: get_user" << dendl;
    //User * ret = make_unique<TracerUser>(this, u, std::move(realStore->get_user(u)))
    return realStore->get_user(u);
  }

  int TracerDriver::get_user_by_access_key(const DoutPrefixProvider *dpp, const std::string& key, optional_yield y, std::unique_ptr<User>* user)
  {
    /*
      RGWUserInfo uinfo;
      User *u;
      int ret = 0;
    */
      /*this is where the objv_tracer would be - Dan P */

      //Some sort of control function [ctl()?]

    ldpp_dout(dpp,20) << "Operation: get_user_by_access_key, key: " << key << dendl;
    int r;
    r = realStore->get_user_by_access_key(dpp, key, y, user);
    ldpp_dout(dpp,20) << "Real store returned from operation: get_user_by_access_key" << dendl;
    return r;
  }

  int TracerDriver::get_user_by_email(const DoutPrefixProvider *dpp, const std::string& email, optional_yield y, std::unique_ptr<User>* user)
  {
    return realStore->get_user_by_email(dpp, email, y, user);
  }

  int TracerDriver::get_user_by_swift(const DoutPrefixProvider *dpp, const std::string& user_str, optional_yield y, std::unique_ptr<User>* user)
  {
    /* Swift keys and subusers are not supported for now */
    return realStore->get_user_by_swift(dpp, user_str, y, user);
  }

  std::string TracerDriver::get_cluster_id(const DoutPrefixProvider* dpp,  optional_yield y)
  {
    return realStore->get_cluster_id(dpp, y); // for instance unique identifier
  }

  std::unique_ptr<Object> TracerDriver::get_object(const rgw_obj_key& k)
  {
    dout(20) << "TRACER: recieved operation: get_object" << dendl;
    return realStore->get_object(k);
  }

   int TracerDriver::get_bucket(const DoutPrefixProvider *dpp, User* u, const rgw_bucket& b, std::unique_ptr<Bucket>* bucket, optional_yield y)
  {
    int ret;
    dout(20) << "TRACER: intercepting operation: get_bucket type 1, from store: " << this->get_name() << dendl;
    ret = realStore->get_bucket(dpp, u, b, bucket, y);
    dout(20) << "TRACER: Returned from get_bucket" << dendl;
    return ret;
  }

  int TracerDriver::get_bucket(User* u, const RGWBucketInfo& i, std::unique_ptr<Bucket>* bucket)
  {
    int ret;
    dout(20) << "TRACER: intercepting operation: get_bucket type 2, from store: " << this->get_name() << dendl;
    ret = realStore->get_bucket(u, i, bucket);
    dout(20) << "TRACER: Returned from get_bucket" << dendl;
    return ret;
  }

  int TracerDriver::get_bucket(const DoutPrefixProvider *dpp, User* u, const std::string& tenant, const std::string& name, std::unique_ptr<Bucket>* bucket, optional_yield y)
  {
    int ret;
    dout(20) << "TRACER: intercepting operation: get_bucket type 3, from store: " << this->get_name() << dendl;
    ret = realStore->get_bucket(dpp, u, tenant, name, std::move(bucket), y);
    return ret;
  }

  bool TracerDriver::is_meta_master()
  {
    return realStore->is_meta_master();
  }

  int TracerDriver::forward_request_to_master(const DoutPrefixProvider *dpp, User* user, obj_version *objv,
      bufferlist& in_data,
      JSONParser *jp, req_info& info,
      optional_yield y)
  {
    return realStore->forward_request_to_master(dpp, user, objv, in_data, jp, info, y);
  }

    std::string TracerDriver::zone_unique_id(uint64_t unique_num)
  {
    return realStore->zone_unique_id(unique_num);
  }

  std::string TracerDriver::zone_unique_trans_id(const uint64_t unique_num)
  {
    return realStore->zone_unique_trans_id(unique_num);
  }

  int TracerDriver::cluster_stat(RGWClusterStat& stats)
  {
    return realStore->cluster_stat(stats);
  }

  std::unique_ptr<Lifecycle> TracerDriver::get_lifecycle(void)
  {
    return realStore->get_lifecycle();
  }

  std::unique_ptr<Completions> TracerDriver::get_completions(void)
  {
    return realStore->get_completions();
  }
  
  std::unique_ptr<Notification> TracerDriver::get_notification(
    rgw::sal::Object* obj, rgw::sal::Object* src_obj, struct req_state* s,
    rgw::notify::EventType event_type, const std::string* object_name)
  {
    return realStore->get_notification(obj, src_obj, s, event_type, object_name);
  }

  std::unique_ptr<Notification> TracerDriver::get_notification(
    const DoutPrefixProvider* dpp, rgw::sal::Object* obj,
    rgw::sal::Object* src_obj, RGWObjectCtx* rctx,
    rgw::notify::EventType event_type, rgw::sal::Bucket* _bucket,
    std::string& _user_id, std::string& _user_tenant, std::string& _req_id,
    optional_yield y)
  {
    return realStore->get_notification(dpp, obj, src_obj, rctx, event_type, _bucket, _user_id, _user_tenant, _req_id, y);
  }

  RGWLC* TracerDriver::get_rgwlc(void) {
    return realStore->get_rgwlc();
  }

  int TracerDriver::log_usage(const DoutPrefixProvider *dpp, map<rgw_user_bucket, RGWUsageBatch>& usage_info)
  {
    return realStore->log_usage(dpp, usage_info);
  }

  int TracerDriver::log_op(const DoutPrefixProvider *dpp, string& oid, bufferlist& bl)
  {
    return realStore->log_op(dpp, oid, bl);
  }

  int TracerDriver::register_to_service_map(const DoutPrefixProvider *dpp, const string& daemon_type,
      const map<string, string>& meta)
  {
    return realStore->register_to_service_map(dpp, daemon_type, meta);
  }

  void TracerDriver::get_ratelimit(RGWRateLimitInfo& bucket_ratelimit, RGWRateLimitInfo& user_ratelimit, RGWRateLimitInfo& anon_ratelimit)
  {
    return realStore->get_ratelimit(bucket_ratelimit, user_ratelimit, anon_ratelimit);
  }

  void TracerDriver::get_quota(RGWQuotaInfo& bucket_quota, RGWQuotaInfo& user_quota)
  {
    return realStore->get_quota(bucket_quota, user_quota);
  }

  int TracerDriver::set_buckets_enabled(const DoutPrefixProvider *dpp, vector<rgw_bucket>& buckets, bool enabled)
  {
    return realStore->set_buckets_enabled(dpp, buckets, enabled);
  }

  int TracerDriver::get_sync_policy_handler(const DoutPrefixProvider *dpp,
      std::optional<rgw_zone_id> zone,
      std::optional<rgw_bucket> bucket,
      RGWBucketSyncPolicyHandlerRef *phandler,
      optional_yield y)
  {
    return realStore->get_sync_policy_handler(dpp, zone, bucket, phandler, y);
  }

  RGWDataSyncStatusManager* TracerDriver::get_data_sync_manager(const rgw_zone_id& source_zone)
  {
    return realStore->get_data_sync_manager(source_zone);
  }

  int TracerDriver::read_all_usage(const DoutPrefixProvider *dpp, uint64_t start_epoch, uint64_t end_epoch, 
      uint32_t max_entries, bool *is_truncated,
      RGWUsageIter& usage_iter,
      map<rgw_user_bucket, rgw_usage_log_entry>& usage)
  {
    return realStore->read_all_usage(dpp, start_epoch, end_epoch, max_entries, is_truncated, usage_iter, usage);
  }

  int TracerDriver::trim_all_usage(const DoutPrefixProvider *dpp, uint64_t start_epoch, uint64_t end_epoch)
  {
    return realStore->trim_all_usage(dpp, start_epoch, end_epoch);
  }

  int TracerDriver::get_config_key_val(string name, bufferlist *bl)
  {
    return realStore->get_config_key_val(name, bl);
  }

  int TracerDriver::meta_list_keys_init(const DoutPrefixProvider *dpp, const string& section, const string& marker, void** phandle)
  {
    return realStore->meta_list_keys_init(dpp, section, marker, phandle);
  }

  int TracerDriver::meta_list_keys_next(const DoutPrefixProvider *dpp, void* handle, int max, list<string>& keys, bool* truncated)
  {
    return realStore->meta_list_keys_next(dpp, handle, max, keys, truncated);
  }

  void TracerDriver::meta_list_keys_complete(void* handle)
  {
    return realStore->meta_list_keys_complete(handle);
  }

  std::string TracerDriver::meta_get_marker(void* handle)
  {
    return realStore->meta_get_marker(handle);
  }

  int TracerDriver::meta_remove(const DoutPrefixProvider *dpp, string& metadata_key, optional_yield y)
  {
    return realStore->meta_remove(dpp, metadata_key, y);
  }

  std::unique_ptr<LuaScriptManager> TracerDriver::get_lua_script_manager()
  {
    return realStore->get_lua_script_manager();
  }

  std::unique_ptr<RGWRole> TracerDriver::get_role(std::string name,
      std::string tenant,
      std::string path,
      std::string trust_policy,
      std::string max_session_duration_str,
      std::multimap<std::string,std::string> tags)
  {
    return realStore->get_role(name, tenant, path, trust_policy, max_session_duration_str, tags);
  }

  std::unique_ptr<RGWRole> TracerDriver::get_role(std::string id)
  {
    return realStore->get_role(id);
  }

  int TracerDriver::get_roles(const DoutPrefixProvider *dpp,
      optional_yield y,
      const std::string& path_prefix,
      const std::string& tenant,
      vector<std::unique_ptr<RGWRole>>& roles)
  {
    return realStore->get_roles(dpp, y, path_prefix, tenant, roles);
  }

  std::unique_ptr<RGWOIDCProvider> TracerDriver::get_oidc_provider()
  {
    return realStore->get_oidc_provider();
  }

    int TracerDriver::get_oidc_providers(const DoutPrefixProvider *dpp,
      const std::string& tenant,
      vector<std::unique_ptr<RGWOIDCProvider>>& providers)
  {
    return realStore->get_oidc_providers(dpp, tenant, providers);
  }

  std::unique_ptr<Writer> TracerDriver::get_append_writer(const DoutPrefixProvider *dpp,
				  optional_yield y,
				  std::unique_ptr<rgw::sal::Object> _head_obj,
				  const rgw_user& owner, RGWObjectCtx& obj_ctx,
				  const rgw_placement_rule *ptail_placement_rule,
				  const std::string& unique_tag,
				  uint64_t position,
				  uint64_t *cur_accounted_size) {
    return realStore->get_append_writer(dpp, y, std::move(_head_obj), owner, obj_ctx, ptail_placement_rule, unique_tag, position, cur_accounted_size);
  }

    std::unique_ptr<Writer> TracerDriver::get_atomic_writer(const DoutPrefixProvider *dpp,
				  optional_yield y,
				  std::unique_ptr<rgw::sal::Object> _head_obj,
				  const rgw_user& owner, RGWObjectCtx& obj_ctx,
				  const rgw_placement_rule *ptail_placement_rule,
				  uint64_t olh_epoch,
				  const std::string& unique_tag) {
    return realStore->get_atomic_writer(dpp, y, std::move(_head_obj), owner, obj_ctx, ptail_placement_rule, olh_epoch, unique_tag);
  }

  void TracerDriver::finalize(void)
  {
      if(realStore)
      realStore->finalize(); //May need to implement additional cleanup for this store itself.
  }



 } //namespace rgw::sal

 extern "C" {

    void* newTracer(const DoutPrefixProvider *dpp, rgw::sal::Store* inputStore) /*takes in a store and wraps */ //may need to also feed in a string for either rados or dbstore. Dan P
    {
      
        rgw::sal::TracerDriver *trace = new rgw::sal::TracerDriver(); //TODO: make sure that the constructor is ready. Dan P
        trace->initialize(inputStore);
        if (trace) {
            ldpp_dout(dpp, 0) << "TracerDriver initialized, intercepting traffic to store name: " << trace->get_name() << dendl; 
            return trace;
        }
        ldpp_dout(dpp, 0) << "ERROR: TracerDriver failed to link to store" << dendl;
        return NULL;
    }

 }