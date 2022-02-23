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


/*includes need to cover both dbstore and rados? Dan P*/
/*Everywhere that there is a TStore pointer may need to be switched
* to a default store pointer. Dan P */


#pragma once

//shared includes
#include "rgw_sal.h"
#include "rgw_oidc_provider.h"
#include "rgw_role.h"
#include "rgw_multi.h"

//rados includes
/*
#include "rgw_multi.h"
#include "rgw_putobj_processor.h"
#include "services/svc_tier_rados.h"
#include "cls/lock/cls_lock_client.h"
*/
//dbstore includes
/*
#include "rgw_lc.h"
#include "store/dbstore/common/dbstore.h"
#include "store/dbstore/dbstore_mgr.h"
*/

//TODO: There's several classes that use DB::Object 

namespace rgw { namespace sal {

  class TracerDriver;

  class LCTSerializer : public LCSerializer {
  const std::string oid;

  public:
    LCTSerializer(TracerDriver* trace, const std::string& oid, const std::string& lock_name, const std::string& cookie) {}

    virtual int try_lock(const DoutPrefixProvider *dpp, utime_t dur, optional_yield y) override { return 0; }
    virtual int unlock() override {
      return 0;
    }
  };

  class TracerLifecycle : public Lifecycle {
  TracerDriver* trace;

  public:
    TracerLifecycle(TracerDriver* _st) : trace(_st) {}

    virtual int get_entry(const std::string& oid, const std::string& marker, LCEntry& entry) override;
    virtual int get_next_entry(const std::string& oid, std::string& marker, LCEntry& entry) override;
    virtual int set_entry(const std::string& oid, const LCEntry& entry) override;
    virtual int list_entries(const std::string& oid, const std::string& marker,
	  		   uint32_t max_entries, std::vector<LCEntry>& entries) override;
    virtual int rm_entry(const std::string& oid, const LCEntry& entry) override;
    virtual int get_head(const std::string& oid, LCHead& head) override;
    virtual int put_head(const std::string& oid, const LCHead& head) override;
    virtual LCSerializer* get_serializer(const std::string& lock_name, const std::string& oid, const std::string& cookie) override;
    };
    
  class TZone : public Zone {
    protected:
      TracerDriver* trace;
      RGWRealm *realm{nullptr};
      RGWZoneGroup *zonegroup{nullptr};
      RGWZone *zone_public_config{nullptr}; /* external zone params, e.g., entrypoints, log flags, etc. */  
      RGWZoneParams *zone_params{nullptr}; /* internal zone params, e.g., rados pools */
      RGWPeriod *current_period{nullptr};
      rgw_zone_id cur_zone_id;
      Zone* realZone;
      //A lot of these can probably be removed - Dan P

    public:
      TZone(TracerDriver* _tracer) : trace(_tracer) {
        realm = new RGWRealm();
        zonegroup = new RGWZoneGroup();
        zone_public_config = new RGWZone();
        zone_params = new RGWZoneParams();
        current_period = new RGWPeriod();
        cur_zone_id = rgw_zone_id(zone_params->get_id());

        // XXX: only default and STANDARD supported for now
        RGWZonePlacementInfo info;
        RGWZoneStorageClasses sc;
        sc.set_storage_class("STANDARD", nullptr, nullptr);
        info.storage_classes = sc;
        zone_params->placement_pools["default"] = info;
      }
      ~TZone() = default;

      virtual const RGWZoneGroup& get_zonegroup() override;
      virtual int get_zonegroup(const std::string& id, RGWZoneGroup& zonegroup) override;
      virtual const RGWZoneParams& get_params() override;
      virtual const rgw_zone_id& get_id() override;
      virtual const RGWRealm& get_realm() override;
      virtual const std::string& get_name() const override;
      virtual bool is_writeable() override;
      virtual bool get_redirect_endpoint(std::string* endpoint) override;
      virtual bool has_zonegroup_api(const std::string& api) const override;
      virtual const std::string& get_current_period_id() override;
  };

  class TNotification : public Notification {
  protected:
  public:
    TNotification(Object* _obj, Object* _src_obj, rgw::notify::EventType _type)
    : Notification(_obj, _src_obj, _type) {}
    ~TNotification() = default;

    virtual int publish_reserve(const DoutPrefixProvider *dpp, RGWObjTags* obj_tags = nullptr) override { return 0;}
    virtual int publish_commit(const DoutPrefixProvider* dpp, uint64_t size,
			       const ceph::real_time& mtime, const std::string& etag, const std::string& version) override { return 0; }
};

class TracerUser : public User {
    private:
      TracerDriver *trace;
      std::unique_ptr<User> realUser;
    public:
      TracerUser(TracerDriver *_st, const rgw_user& _u, std::unique_ptr<User> _ru) : User(_u), trace(_st), realUser(std::move(_ru)) { }
      TracerUser(TracerDriver *_st, const RGWUserInfo& _i, std::unique_ptr<User> _ru) : User(_i), trace(_st), realUser(std::move(_ru)) { }
      TracerUser(TracerDriver * _st, const RGWUserInfo& _i, std::unique_ptr<User>* _ru) : User(_i), trace(_st),  realUser(std::move(* _ru)) {}
      TracerUser(TracerDriver *_st) : trace(_st) { }
      TracerUser(TracerUser& _o, std::unique_ptr<User> _ru) : realUser(std::move(_ru)) {}
      TracerUser() {}

      virtual std::unique_ptr<User> clone() override {
        return std::unique_ptr<User>(new TracerUser(*this, std::move(this->realUser)));
      }

      int list_buckets(const DoutPrefixProvider *dpp, const std::string& marker, const std::string& end_marker,
          uint64_t max, bool need_stats, BucketList& buckets, optional_yield y) override;
      virtual int create_bucket(const DoutPrefixProvider* dpp,
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
          optional_yield y) override;
      virtual int read_attrs(const DoutPrefixProvider* dpp, optional_yield y) override;
      virtual int read_stats(const DoutPrefixProvider *dpp,
          optional_yield y, RGWStorageStats* stats,
          ceph::real_time *last_stats_sync = nullptr,
          ceph::real_time *last_stats_update = nullptr) override;
      virtual int read_stats_async(const DoutPrefixProvider *dpp, RGWGetUserStats_CB* cb) override;
      virtual int complete_flush_stats(const DoutPrefixProvider *dpp, optional_yield y) override;
      virtual int read_usage(const DoutPrefixProvider *dpp, uint64_t start_epoch, uint64_t end_epoch, uint32_t max_entries,
          bool* is_truncated, RGWUsageIter& usage_iter,
          map<rgw_user_bucket, rgw_usage_log_entry>& usage) override;
      virtual int trim_usage(const DoutPrefixProvider *dpp, uint64_t start_epoch, uint64_t end_epoch) override;

      /* Placeholders */
      virtual int merge_and_store_attrs(const DoutPrefixProvider* dpp, Attrs& new_attrs, optional_yield y) override;
      virtual int load_user(const DoutPrefixProvider* dpp, optional_yield y) override;
      virtual int store_user(const DoutPrefixProvider* dpp, optional_yield y, bool exclusive, RGWUserInfo* old_info = nullptr) override;
      virtual int remove_user(const DoutPrefixProvider* dpp, optional_yield y) override;

      friend class TracerBucket;
  };

  class TracerBucket : public Bucket {
    private:
      TracerDriver *trace;
      RGWAccessControlPolicy acls;
      std::unique_ptr<Bucket> realBucket;
      

    public:
      
      TracerBucket(TracerBucket &_b, std::unique_ptr<Bucket> _rb) 
      : trace(_b.trace),
        acls(), 
        realBucket(std::move(_rb)) 
      {}
      
      TracerBucket(TracerDriver *_st)
        : trace(_st),
        acls() {
        }

      TracerBucket(TracerDriver *_st, User* _u)
        : Bucket(_u),
        trace(_st),
        acls() {
        }

      TracerBucket(TracerDriver *_st, const rgw_bucket& _b)
        : Bucket(_b),
        trace(_st),
        acls() {
        }

      TracerBucket(TracerDriver *_st, const RGWBucketEnt& _e)
        : Bucket(_e),
        trace(_st),
        acls() {
        }

      TracerBucket(TracerDriver *_st, const RGWBucketInfo& _i)
        : Bucket(_i),
        trace(_st),
        acls() {
        }

        TracerBucket(TracerDriver *_st, const RGWBucketInfo& _i, std::unique_ptr<Bucket> * _rb)
        : Bucket(_i),
        trace(_st),
        acls(),
        realBucket(std::move(* _rb)) {
        }

      TracerBucket(TracerDriver *_st, const rgw_bucket& _b, User* _u)
        : Bucket(_b, _u),
        trace(_st),
        acls()
        {
        }

      TracerBucket(TracerDriver *_st, const RGWBucketEnt& _e, User* _u)
        : Bucket(_e, _u),
        trace(_st),
        acls()
        {
        }

      TracerBucket(TracerDriver *_st, const RGWBucketInfo& _i, User* _u)
        : Bucket(_i, _u),
        trace(_st),
        acls() {
        }

      ~TracerBucket() { }

      virtual std::unique_ptr<Bucket> clone() override {
        return std::unique_ptr<Bucket>(new TracerBucket(*this, std::move(this->realBucket)));
      }
      virtual std::unique_ptr<Object> get_object(const rgw_obj_key& k) override;
      virtual int list(const DoutPrefixProvider *dpp, ListParams&, int, ListResults&, optional_yield y) override;
      virtual int remove_bucket(const DoutPrefixProvider *dpp, bool delete_children, bool forward_to_master, req_info* req_info, optional_yield y) override;
      virtual int remove_bucket_bypass_gc(int concurrent_max, bool
					keep_index_consistent,
					optional_yield y, const
					DoutPrefixProvider *dpp) override;
      virtual RGWAccessControlPolicy& get_acl(void) override { return acls; }
      virtual int set_acl(const DoutPrefixProvider *dpp, RGWAccessControlPolicy& acl, optional_yield y) override;
      virtual int load_bucket(const DoutPrefixProvider *dpp, optional_yield y, bool get_stats = false) override;
      virtual int read_stats(const DoutPrefixProvider *dpp, int shard_id,
          std::string *bucket_ver, std::string *master_ver,
          std::map<RGWObjCategory, RGWStorageStats>& stats,
          std::string *max_marker = nullptr,
          bool *syncstopped = nullptr) override;
      virtual int read_stats_async(const DoutPrefixProvider *dpp, int shard_id, RGWGetBucketStats_CB* ctx) override;
      virtual int sync_user_stats(const DoutPrefixProvider *dpp, optional_yield y) override;
      virtual int update_container_stats(const DoutPrefixProvider *dpp) override;
      virtual int check_bucket_shards(const DoutPrefixProvider *dpp) override;
      virtual int chown(const DoutPrefixProvider *dpp, User* new_user, User* old_user, optional_yield y, const std::string* marker = nullptr) override;
      virtual int put_info(const DoutPrefixProvider *dpp, bool exclusive, ceph::real_time mtime) override;
      virtual bool is_owner(User* user) override;
      virtual int check_empty(const DoutPrefixProvider *dpp, optional_yield y) override;
      virtual int check_quota(const DoutPrefixProvider *dpp, RGWQuotaInfo& user_quota, RGWQuotaInfo& bucket_quota, uint64_t obj_size, optional_yield y, bool check_size_only = false) override;
      virtual int merge_and_store_attrs(const DoutPrefixProvider *dpp, Attrs& attrs, optional_yield y) override;
      virtual int try_refresh_info(const DoutPrefixProvider *dpp, ceph::real_time *pmtime) override;
      virtual int read_usage(const DoutPrefixProvider *dpp, uint64_t start_epoch, uint64_t end_epoch, uint32_t max_entries,
          bool *is_truncated, RGWUsageIter& usage_iter,
          map<rgw_user_bucket, rgw_usage_log_entry>& usage) override;
      virtual int trim_usage(const DoutPrefixProvider *dpp, uint64_t start_epoch, uint64_t end_epoch) override;
      virtual int remove_objs_from_index(const DoutPrefixProvider *dpp, std::list<rgw_obj_index_key>& objs_to_unlink) override;
      virtual int check_index(const DoutPrefixProvider *dpp, std::map<RGWObjCategory, RGWStorageStats>& existing_stats, std::map<RGWObjCategory, RGWStorageStats>& calculated_stats) override;
      virtual int rebuild_index(const DoutPrefixProvider *dpp) override;
      virtual int set_tag_timeout(const DoutPrefixProvider *dpp, uint64_t timeout) override;
      virtual int purge_instance(const DoutPrefixProvider *dpp) override;
      virtual std::unique_ptr<MultipartUpload> get_multipart_upload(
				const std::string& oid, std::optional<std::string> upload_id,
				ACLOwner owner={}, ceph::real_time mtime=ceph::real_clock::now()) override;
      virtual int list_multiparts(const DoutPrefixProvider *dpp,
				const string& prefix,
				string& marker,
				const string& delim,
				const int& max_uploads,
				vector<std::unique_ptr<MultipartUpload>>& uploads,
				map<string, bool> *common_prefixes,
				bool *is_truncated) override;
      virtual int abort_multiparts(const DoutPrefixProvider* dpp,
				   CephContext* cct) override;

      friend class TracerDriver;
  };

  class TLuaScriptManager : public LuaScriptManager {
    TracerDriver* trace;

    public:
    TLuaScriptManager(TracerDriver* _s) : trace(_s)
    {
    }
    virtual ~TLuaScriptManager() = default;

    virtual int get(const DoutPrefixProvider* dpp, optional_yield y, const std::string& key, std::string& script) override { return -ENOENT; }
    virtual int put(const DoutPrefixProvider* dpp, optional_yield y, const std::string& key, const std::string& script) override { return -ENOENT; }
    virtual int del(const DoutPrefixProvider* dpp, optional_yield y, const std::string& key) override { return -ENOENT; }
  };

  class TOIDCProvider : public RGWOIDCProvider {
    TracerDriver* trace;
    public:
    TOIDCProvider(TracerDriver* _store) : trace(_store) {}
    ~TOIDCProvider() = default;

    virtual int store_url(const DoutPrefixProvider *dpp, const std::string& url, bool exclusive, optional_yield y) override { return 0; }
    virtual int read_url(const DoutPrefixProvider *dpp, const std::string& url, const std::string& tenant) override { return 0; }
    virtual int delete_obj(const DoutPrefixProvider *dpp, optional_yield y) override { return 0;}

  };

  /*
   * For multipart upload, below is the process flow -
   *
   * MultipartUpload::Init - create head object of meta obj (src_obj_name + "." + upload_id)
   *                     [ Meta object stores all the parts upload info]
   * MultipartWriter::process - create all data/tail objects with obj_name same as
   *                        meta obj (so that they can all be identified & deleted 
   *                        during abort)
   * MultipartUpload::Abort - Just delete meta obj .. that will indirectly delete all the
   *                     uploads associated with that upload id / meta obj so far.
   * MultipartUpload::Complete - create head object of the original object (if not exists) &
   *                     rename all data/tail objects to orig object name and update
   *                     metadata of the orig object.
   */
  class TMultipartPart : public MultipartPart {
  protected:
    RGWUploadPartInfo info; /* XXX: info contains manifest also which is not needed */

  public:
    TMultipartPart() = default;
    virtual ~TMultipartPart() = default;

    virtual RGWUploadPartInfo& get_info() { return info; }
    virtual void set_info(const RGWUploadPartInfo& _info) { info = _info; }
    virtual uint32_t get_num() { return info.num; }
    virtual uint64_t get_size() { return info.accounted_size; }
    virtual const std::string& get_etag() { return info.etag; }
    virtual ceph::real_time& get_mtime() { return info.modified; }

  };

  class TracerMPObj {
    std::string oid; // object name
    std::string upload_id;
    std::string meta; // multipart meta object = <oid>.<upload_id>
  public:
    TracerMPObj() {}
    TracerMPObj(const std::string& _oid, const std::string& _upload_id) {
      init(_oid, _upload_id, _upload_id);
    }
    TracerMPObj(const std::string& _oid, std::optional<std::string> _upload_id) {
      if (_upload_id) {
        init(_oid, *_upload_id, *_upload_id);
      } else {
        from_meta(_oid);
      }
    }
    void init(const std::string& _oid, const std::string& _upload_id) {
      init(_oid, _upload_id, _upload_id);
    }
    void init(const std::string& _oid, const std::string& _upload_id, const std::string& part_unique_str) {
      if (_oid.empty()) {
        clear();
        return;
      }
      oid = _oid;
      upload_id = _upload_id;
      meta = oid + "." + upload_id;
    }
    const std::string& get_upload_id() const {
      return upload_id;
    }
    const std::string& get_key() const {
      return oid;
    }
    const std::string& get_meta() const { return meta; }
    bool from_meta(const std::string& meta) {
      int end_pos = meta.length();
      int mid_pos = meta.rfind('.', end_pos - 1); // <key>.<upload_id>
      if (mid_pos < 0)
        return false;
      oid = meta.substr(0, mid_pos);
      upload_id = meta.substr(mid_pos + 1, end_pos - mid_pos - 1);
      init(oid, upload_id, upload_id);
      return true;
    }
    void clear() {
      oid = "";
      meta = "";
      upload_id = "";
    }
  };

  class TracerMultipartUpload : public MultipartUpload {
    TracerDriver* trace;
    TracerMPObj mp_obj;
    ACLOwner owner;
    ceph::real_time mtime;
    rgw_placement_rule placement;

  public:
    TracerMultipartUpload(TracerDriver* _store, Bucket* _bucket, const std::string& oid, std::optional<std::string> upload_id, ACLOwner _owner, ceph::real_time _mtime) : MultipartUpload(_bucket), trace(_store), mp_obj(oid, upload_id), owner(_owner), mtime(_mtime) {}
    virtual ~TracerMultipartUpload() = default;

    virtual const std::string& get_meta() const { return mp_obj.get_meta(); }
    virtual const std::string& get_key() const { return mp_obj.get_key(); }
    virtual const std::string& get_upload_id() const { return mp_obj.get_upload_id(); }
    virtual const ACLOwner& get_owner() const override { return owner; }
    virtual ceph::real_time& get_mtime() { return mtime; }
    virtual std::unique_ptr<rgw::sal::Object> get_meta_obj() override;
    virtual int init(const DoutPrefixProvider* dpp, optional_yield y, RGWObjectCtx* obj_ctx, ACLOwner& owner, rgw_placement_rule& dest_placement, rgw::sal::Attrs& attrs) override;
    virtual int list_parts(const DoutPrefixProvider* dpp, CephContext* cct,
			 int num_parts, int marker,
			 int* next_marker, bool* truncated,
			 bool assume_unsorted = false) override;
    virtual int abort(const DoutPrefixProvider* dpp, CephContext* cct,
		    RGWObjectCtx* obj_ctx) override;
    virtual int complete(const DoutPrefixProvider* dpp,
		       optional_yield y, CephContext* cct,
		       std::map<int, std::string>& part_etags,
		       std::list<rgw_obj_index_key>& remove_objs,
		       uint64_t& accounted_size, bool& compressed,
		       RGWCompressionInfo& cs_info, off_t& ofs,
		       std::string& tag, ACLOwner& owner,
		       uint64_t olh_epoch,
		       rgw::sal::Object* target_obj,
		       RGWObjectCtx* obj_ctx) override;
    virtual int get_info(const DoutPrefixProvider *dpp, optional_yield y, RGWObjectCtx* obj_ctx, rgw_placement_rule** rule, rgw::sal::Attrs* attrs = nullptr) override;
    virtual std::unique_ptr<Writer> get_writer(const DoutPrefixProvider *dpp,
			  optional_yield y,
			  std::unique_ptr<rgw::sal::Object> _head_obj,
			  const rgw_user& owner, RGWObjectCtx& obj_ctx,
			  const rgw_placement_rule *ptail_placement_rule,
			  uint64_t part_num,
			  const std::string& part_num_str) override;
  };

  class TracerMultipartPart : public MultipartPart {
  protected:
    RGWUploadPartInfo info; /* XXX: info contains manifest also which is not needed */

  public:
    TracerMultipartPart() = default;
    virtual ~TracerMultipartPart() = default;

    virtual RGWUploadPartInfo& get_info() { return info; }
    virtual void set_info(const RGWUploadPartInfo& _info) { info = _info; }
    virtual uint32_t get_num() { return info.num; }
    virtual uint64_t get_size() { return info.accounted_size; }
    virtual const std::string& get_etag() { return info.etag; }
    virtual ceph::real_time& get_mtime() { return info.modified; }

  };

    class TObject : public Object {
    private:
      std::unique_ptr<Object> realObject;
      TracerDriver* trace;
      RGWAccessControlPolicy acls;
      /* XXX: to be removed. Till Dan's patch comes, a placeholder
       * for RGWObjState
       */
      RGWObjState state;

    public:
      struct TReadOp : public ReadOp {
        private:
          ReadOp* realReadOp;
          TObject* source;
          RGWObjectCtx* rctx;
          //DB::Object op_target;
          //DB::Object::Read parent_op; //leaving the DB:: here because it won't compile otherwise

        public:
          TReadOp(TObject *_source, RGWObjectCtx *_rctx);

          virtual int prepare(optional_yield y, const DoutPrefixProvider* dpp) override;
          virtual int read(int64_t ofs, int64_t end, bufferlist& bl, optional_yield y, const DoutPrefixProvider* dpp) override;
          virtual int iterate(const DoutPrefixProvider* dpp, int64_t ofs, int64_t end, RGWGetDataCB* cb, optional_yield y) override;
          virtual int get_attr(const DoutPrefixProvider* dpp, const char* name, bufferlist& dest, optional_yield y) override; 
      };

      struct TDeleteOp : public DeleteOp {
        private:
          DeleteOp* realDeleteOp;
          TObject* source;
          RGWObjectCtx* rctx;
          //DB::Object op_target;
          //DB::Object::Delete parent_op;

        public:
          TDeleteOp(TObject* _source, RGWObjectCtx* _rctx);

          virtual int delete_obj(const DoutPrefixProvider* dpp, optional_yield y) override;
      };

      TObject() = default;

      TObject(TracerDriver*_st, const rgw_obj_key& _k)
        : Object(_k),
        trace(_st),
        acls() {}

      TObject(TracerDriver*_st, const rgw_obj_key& _k, Bucket* _b)
        : Object(_k, _b),
        trace(_st),
        acls() {}

      TObject(TObject &_o, std::unique_ptr<Object> _ro)
      : realObject(std::move(_ro)){}

        /*TODO: implement new constructor that takes a real object in - Dan P*/

      TObject(TObject& _o) = default;

      virtual int delete_object(const DoutPrefixProvider* dpp,
          RGWObjectCtx* obj_ctx,
          optional_yield y,
          bool prevent_versioning = false) override;
      virtual int delete_obj_aio(const DoutPrefixProvider* dpp, RGWObjState* astate, Completions* aio,
          bool keep_index_consistent, optional_yield y) override;
      virtual int copy_object(RGWObjectCtx& obj_ctx, User* user,
          req_info* info, const rgw_zone_id& source_zone,
          rgw::sal::Object* dest_object, rgw::sal::Bucket* dest_bucket,
          rgw::sal::Bucket* src_bucket,
          const rgw_placement_rule& dest_placement,
          ceph::real_time* src_mtime, ceph::real_time* mtime,
          const ceph::real_time* mod_ptr, const ceph::real_time* unmod_ptr,
          bool high_precision_time,
          const char* if_match, const char* if_nomatch,
          AttrsMod attrs_mod, bool copy_if_newer, Attrs& attrs,
          RGWObjCategory category, uint64_t olh_epoch,
          boost::optional<ceph::real_time> delete_at,
          std::string* version_id, std::string* tag, std::string* etag,
          void (*progress_cb)(off_t, void *), void* progress_data,
          const DoutPrefixProvider* dpp, optional_yield y) override;
      virtual RGWAccessControlPolicy& get_acl(void) override { return acls; }
      virtual int set_acl(const RGWAccessControlPolicy& acl) override { acls = acl; return 0; }
      virtual void set_atomic(RGWObjectCtx* rctx) const override;
      virtual void set_prefetch_data(RGWObjectCtx* rctx) override;
      virtual void set_compressed(RGWObjectCtx* rctx) override;

      virtual int get_obj_state(const DoutPrefixProvider* dpp, RGWObjectCtx* rctx, RGWObjState **state, optional_yield y, bool follow_olh = true) override;
      virtual int set_obj_attrs(const DoutPrefixProvider* dpp, RGWObjectCtx* rctx, Attrs* setattrs, Attrs* delattrs, optional_yield y, rgw_obj* target_obj = NULL) override;
      virtual int get_obj_attrs(RGWObjectCtx* rctx, optional_yield y, const DoutPrefixProvider* dpp, rgw_obj* target_obj = NULL) override;
      virtual int modify_obj_attrs(RGWObjectCtx* rctx, const char* attr_name, bufferlist& attr_val, optional_yield y, const DoutPrefixProvider* dpp) override;
      virtual int delete_obj_attrs(const DoutPrefixProvider* dpp, RGWObjectCtx* rctx, const char* attr_name, optional_yield y) override;
      virtual bool is_expired() override;
      virtual void gen_rand_obj_instance_name() override;
      virtual std::unique_ptr<Object> clone() override {
        return std::unique_ptr<Object>(new TObject(*this, std::move(this->realObject)));
      }

      virtual MPSerializer* get_serializer(const DoutPrefixProvider *dpp, const std::string& lock_name) override;
      virtual int transition(RGWObjectCtx& rctx,
          Bucket* bucket,
          const rgw_placement_rule& placement_rule,
          const real_time& mtime,
          uint64_t olh_epoch,
          const DoutPrefixProvider* dpp,
          optional_yield y) override;
      virtual bool placement_rules_match(rgw_placement_rule& r1, rgw_placement_rule& r2) override;
      virtual int dump_obj_layout(const DoutPrefixProvider *dpp, optional_yield y, Formatter* f, RGWObjectCtx* obj_ctx) override;

      /* Swift versioning */
      virtual int swift_versioning_restore(RGWObjectCtx* obj_ctx,
          bool& restored,
          const DoutPrefixProvider* dpp) override;
      virtual int swift_versioning_copy(RGWObjectCtx* obj_ctx,
          const DoutPrefixProvider* dpp,
          optional_yield y) override;

      /* OPs */
      virtual std::unique_ptr<ReadOp> get_read_op(RGWObjectCtx *) override;
      virtual std::unique_ptr<DeleteOp> get_delete_op(RGWObjectCtx*) override;

      /* OMAP */
      virtual int omap_get_vals(const DoutPrefixProvider *dpp, const std::string& marker, uint64_t count,
          std::map<std::string, bufferlist> *m,
          bool* pmore, optional_yield y) override;
      virtual int omap_get_all(const DoutPrefixProvider *dpp, std::map<std::string, bufferlist> *m,
          optional_yield y) override;
      virtual int omap_get_vals_by_keys(const DoutPrefixProvider *dpp, const std::string& oid,
          const std::set<std::string>& keys,
          Attrs* vals) override;
      virtual int omap_set_val_by_key(const DoutPrefixProvider *dpp, const std::string& key, bufferlist& val,
          bool must_exist, optional_yield y) override;
  };

  class MPTSerializer : public MPSerializer {

  public:
    MPTSerializer(const DoutPrefixProvider *dpp, TracerDriver* store, TObject* obj, const std::string& lock_name) {}

    virtual int try_lock(const DoutPrefixProvider *dpp, utime_t dur, optional_yield y) override {return 0; }
    virtual int unlock() override { return 0;}
  };

  class TracerAtomicWriter : public Writer {
  protected:
    rgw::sal::TracerDriver* trace;
    const rgw_user& owner;
	  const rgw_placement_rule *ptail_placement_rule;
	  uint64_t olh_epoch;
  	const std::string& unique_tag;
    TObject obj;
    DB::Object op_target; //leaving these as is for compilation
    DB::Object::Write parent_op;
    uint64_t total_data_size = 0; /* for total data being uploaded */
    bufferlist head_data;
    bufferlist tail_part_data;
    uint64_t tail_part_offset;
    uint64_t tail_part_size = 0; /* corresponds to each tail part being
                                  written to dbstore */

    public:
    TracerAtomicWriter(const DoutPrefixProvider *dpp,
	    	    optional_yield y,
		        std::unique_ptr<rgw::sal::Object> _head_obj,
		        DBStore* _store,
    		    const rgw_user& _owner, RGWObjectCtx& obj_ctx,
	    	    const rgw_placement_rule *_ptail_placement_rule,
		        uint64_t _olh_epoch,
		        const std::string& _unique_tag);
    ~TracerAtomicWriter() = default;

    // prepare to start processing object data
    virtual int prepare(optional_yield y) override;

    // Process a bufferlist
    virtual int process(bufferlist&& data, uint64_t offset) override;

    // complete the operation and make its result visible to clients
    virtual int complete(size_t accounted_size, const std::string& etag,
                         ceph::real_time *mtime, ceph::real_time set_mtime,
                         std::map<std::string, bufferlist>& attrs,
                         ceph::real_time delete_at,
                         const char *if_match, const char *if_nomatch,
                         const std::string *user_data,
                         rgw_zone_set *zones_trace, bool *canceled,
                         optional_yield y) override;
  };

  class TracerMultipartWriter : public Writer {
  protected:
    rgw::sal::TracerDriver* trace;
    const rgw_user& owner;
	const rgw_placement_rule *ptail_placement_rule;
	uint64_t olh_epoch;
    std::unique_ptr<rgw::sal::Object> head_obj;
    string upload_id;
    string oid; /* object->name() + "." + "upload_id" + "." + part_num */
    std::unique_ptr<rgw::sal::Object> meta_obj;
    DB::Object op_target;
    DB::Object::Write parent_op;
    int part_num;
    string part_num_str;
    uint64_t total_data_size = 0; /* for total data being uploaded */
    bufferlist head_data;
    bufferlist tail_part_data;
    uint64_t tail_part_offset;
    uint64_t tail_part_size = 0; /* corresponds to each tail part being
                                  written to dbstore */

public:
    TracerMultipartWriter(const DoutPrefixProvider *dpp,
		       optional_yield y, MultipartUpload* upload,
		       std::unique_ptr<rgw::sal::Object> _head_obj,
		       DBStore* _store,
		       const rgw_user& owner, RGWObjectCtx& obj_ctx,
		       const rgw_placement_rule *ptail_placement_rule,
		       uint64_t part_num, const std::string& part_num_str);
    ~TracerMultipartWriter() = default;

    // prepare to start processing object data
    virtual int prepare(optional_yield y) override;

    // Process a bufferlist
    virtual int process(bufferlist&& data, uint64_t offset) override;

    // complete the operation and make its result visible to clients
    virtual int complete(size_t accounted_size, const std::string& etag,
                       ceph::real_time *mtime, ceph::real_time set_mtime,
                       std::map<std::string, bufferlist>& attrs,
                       ceph::real_time delete_at,
                       const char *if_match, const char *if_nomatch,
                       const std::string *user_data,
                       rgw_zone_set *zones_trace, bool *canceled,
                       optional_yield y) override;
  };





  class TracerDriver : public Store {
  private:
    Store* realStore; /* The one that actually does stuff */
    TZone* zone;
    
    

  public:
    TracerDriver() : realStore(nullptr), zone(nullptr) {}
    ~TracerDriver() { delete realStore; }


      void initialize(Store* inputStore)
      {
        this->realStore = inputStore;
      }

      virtual const char* get_name() const override 
      {
        return realStore->get_name();
      }
      virtual std::unique_ptr<User> get_user(const rgw_user& u)  override;
      virtual int get_user_by_access_key(const DoutPrefixProvider *dpp, const std::string& key, optional_yield y, std::unique_ptr<User>* user) override;
      virtual int get_user_by_email(const DoutPrefixProvider *dpp, const std::string& email, optional_yield y, std::unique_ptr<User>* user) override;
      virtual int get_user_by_swift(const DoutPrefixProvider *dpp, const std::string& user_str, optional_yield y, std::unique_ptr<User>* user) override;
      virtual std::unique_ptr<Object> get_object(const rgw_obj_key& k) override;
      virtual std::string get_cluster_id(const DoutPrefixProvider* dpp, optional_yield y) override;
      virtual int get_bucket(const DoutPrefixProvider *dpp, User* u, const rgw_bucket& b, std::unique_ptr<Bucket>* bucket, optional_yield y) override;
      virtual int get_bucket(User* u, const RGWBucketInfo& i, std::unique_ptr<Bucket>* bucket) override;
      virtual int get_bucket(const DoutPrefixProvider *dpp, User* u, const std::string& tenant, const std::string&name, std::unique_ptr<Bucket>* bucket, optional_yield y) override;
      virtual bool is_meta_master() override;
      virtual int forward_request_to_master(const DoutPrefixProvider *dpp, User* user, obj_version* objv,
          bufferlist& in_data, JSONParser *jp, req_info& info,
          optional_yield y) override;
      virtual Zone* get_zone() { return realStore->get_zone(); }
      virtual std::string zone_unique_id(uint64_t unique_num) override;
      virtual std::string zone_unique_trans_id(const uint64_t unique_num) override;
      virtual int cluster_stat(RGWClusterStat& stats) override;
      virtual std::unique_ptr<Lifecycle> get_lifecycle(void) override;
      virtual std::unique_ptr<Completions> get_completions(void) override;

  virtual std::unique_ptr<Notification> get_notification(
    rgw::sal::Object* obj, rgw::sal::Object* src_obj, struct req_state* s,
    rgw::notify::EventType event_type, const std::string* object_name) override;

  virtual std::unique_ptr<Notification> get_notification(
    const DoutPrefixProvider* dpp, rgw::sal::Object* obj,
    rgw::sal::Object* src_obj, RGWObjectCtx* rctx,
    rgw::notify::EventType event_type, rgw::sal::Bucket* _bucket,
    std::string& _user_id, std::string& _user_tenant, std::string& _req_id,
    optional_yield y) override;
    
      virtual RGWLC* get_rgwlc(void) override;
      virtual RGWCoroutinesManagerRegistry* get_cr_registry() override { return NULL; }
      virtual int log_usage(const DoutPrefixProvider *dpp, map<rgw_user_bucket, RGWUsageBatch>& usage_info) override;
      virtual int log_op(const DoutPrefixProvider *dpp, std::string& oid, bufferlist& bl) override;
      virtual int register_to_service_map(const DoutPrefixProvider *dpp, const string& daemon_type,
          const map<string, string>& meta) override;
      virtual void get_ratelimit(RGWRateLimitInfo& bucket_ratelimit, RGWRateLimitInfo& user_ratelimit, RGWRateLimitInfo& anon_ratelimit) override;
      virtual void get_quota(RGWQuotaInfo& bucket_quota, RGWQuotaInfo& user_quota) override;
      virtual int set_buckets_enabled(const DoutPrefixProvider *dpp, vector<rgw_bucket>& buckets, bool enabled) override;
      virtual uint64_t get_new_req_id() override { return 0; }
      virtual int get_sync_policy_handler(const DoutPrefixProvider *dpp,
          std::optional<rgw_zone_id> zone,
          std::optional<rgw_bucket> bucket,
          RGWBucketSyncPolicyHandlerRef *phandler,
          optional_yield y) override;
      virtual RGWDataSyncStatusManager* get_data_sync_manager(const rgw_zone_id& source_zone) override;
      virtual void wakeup_meta_sync_shards(set<int>& shard_ids) override {}; //check this later Dan P
      virtual void wakeup_data_sync_shards(const DoutPrefixProvider *dpp, const rgw_zone_id& source_zone, map<int, set<string> >& shard_ids) override { return; }
      virtual int clear_usage(const DoutPrefixProvider *dpp) override { return 0; }
      virtual int read_all_usage(const DoutPrefixProvider *dpp, uint64_t start_epoch, uint64_t end_epoch,
          uint32_t max_entries, bool *is_truncated,
          RGWUsageIter& usage_iter,
          map<rgw_user_bucket, rgw_usage_log_entry>& usage) override;
      virtual int trim_all_usage(const DoutPrefixProvider *dpp, uint64_t start_epoch, uint64_t end_epoch) override;
      virtual int get_config_key_val(std::string name, bufferlist* bl) override;
      virtual int meta_list_keys_init(const DoutPrefixProvider *dpp, const std::string& section, const std::string& marker, void** phandle) override;
      virtual int meta_list_keys_next(const DoutPrefixProvider *dpp, void* handle, int max, list<std::string>& keys, bool* truncated) override;
      virtual void meta_list_keys_complete(void* handle) override;
      virtual std::string meta_get_marker(void *handle) override;
      virtual int meta_remove(const DoutPrefixProvider *dpp, string& metadata_key, optional_yield y) override;

      virtual const RGWSyncModuleInstanceRef& get_sync_module() { return realStore->get_sync_module(); }
      virtual std::string get_host_id() { return realStore->get_host_id(); }

      virtual std::unique_ptr<LuaScriptManager> get_lua_script_manager() override;
      virtual std::unique_ptr<RGWRole> get_role(std::string name,
          std::string tenant,
          std::string path="",
          std::string trust_policy="",
          std::string max_session_duration_str="",
          std::multimap<std::string,std::string> tags={}) override;
      virtual std::unique_ptr<RGWRole> get_role(std::string id) override;
      virtual int get_roles(const DoutPrefixProvider *dpp,
          optional_yield y,
          const std::string& path_prefix,
          const std::string& tenant,
          vector<std::unique_ptr<RGWRole>>& roles) override;
      virtual std::unique_ptr<RGWOIDCProvider> get_oidc_provider() override;
      virtual int get_oidc_providers(const DoutPrefixProvider *dpp,
          const std::string& tenant,
          vector<std::unique_ptr<RGWOIDCProvider>>& providers) override;
      virtual std::unique_ptr<Writer> get_append_writer(const DoutPrefixProvider *dpp,
				  optional_yield y,
				  std::unique_ptr<rgw::sal::Object> _head_obj,
				  const rgw_user& owner, RGWObjectCtx& obj_ctx,
				  const rgw_placement_rule *ptail_placement_rule,
				  const std::string& unique_tag,
				  uint64_t position,
				  uint64_t *cur_accounted_size) override;
      virtual std::unique_ptr<Writer> get_atomic_writer(const DoutPrefixProvider *dpp,
				  optional_yield y,
				  std::unique_ptr<rgw::sal::Object> _head_obj,
				  const rgw_user& owner, RGWObjectCtx& obj_ctx,
				  const rgw_placement_rule *ptail_placement_rule,
				  uint64_t olh_epoch,
				  const std::string& unique_tag) override;

      virtual void finalize(void) override;

      virtual CephContext *ctx(void) override {
        return realStore->ctx();
      }

      virtual const std::string& get_luarocks_path() const override {
        return realStore->get_luarocks_path();
      }

      virtual void set_luarocks_path(const std::string& path) override {
        realStore->set_luarocks_path(path);
      }
  };

}} //namespace rgw::sal