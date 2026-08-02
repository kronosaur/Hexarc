//	CArcRouteUtil.cpp
//
//	Utilities for Hyperion route configuration shared by Hyperion and CodeSlinger.
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_CREATED_ON,					"createdOn");
DECLARE_CONST_STRING(FIELD_ACTION,						"action");
DECLARE_CONST_STRING(FIELD_HOSTNAME,					"hostname");
DECLARE_CONST_STRING(FIELD_ID,							"id");
DECLARE_CONST_STRING(FIELD_COLUMNS,						"columns");
DECLARE_CONST_STRING(FIELD_KEY,							"key");
DECLARE_CONST_STRING(FIELD_KEY_TYPE,					"keyType");
DECLARE_CONST_STRING(FIELD_MODIFIED_ON,					"modifiedOn");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_OPTIONS,						"options");
DECLARE_CONST_STRING(FIELD_PACKAGE,						"package");
DECLARE_CONST_STRING(FIELD_PORT,						"port");
DECLARE_CONST_STRING(FIELD_PRIMARY_KEY,					"primaryKey");
DECLARE_CONST_STRING(FIELD_PRIORITY,					"priority");
DECLARE_CONST_STRING(FIELD_PROTOCOL,					"protocol");
DECLARE_CONST_STRING(FIELD_ROUTE_ID,					"routeID");
DECLARE_CONST_STRING(FIELD_SERVICE,						"service");
DECLARE_CONST_STRING(FIELD_SECONDARY_VIEWS,				"secondaryViews");
DECLARE_CONST_STRING(FIELD_SOURCE,						"source");
DECLARE_CONST_STRING(FIELD_STATUS,						"status");
DECLARE_CONST_STRING(FIELD_TARGET,						"target");
DECLARE_CONST_STRING(FIELD_UNENCRYPTED_PORT,			"unencryptedPort");
DECLARE_CONST_STRING(FIELD_URL_PATH,					"urlPath");
DECLARE_CONST_STRING(FIELD_X,							"x");

DECLARE_CONST_STRING(KEY_TYPE_UTF8,						"utf8");
DECLARE_CONST_STRING(TABLE_ARC_HOSTS,					"Arc.hosts");
DECLARE_CONST_STRING(TYPENAME_ARC_HOST_SCHEMA,			"ArcHostSchema");
DECLARE_CONST_STRING(TYPENAME_ARC_ROUTE_SCHEMA,			"ArcRouteSchema");
DECLARE_CONST_STRING(VIEW_BY_HOST,						"byHost");
DECLARE_CONST_STRING(VIEW_BY_ACTION,					"byAction");
DECLARE_CONST_STRING(VIEW_BY_PACKAGE,					"byPackage");
DECLARE_CONST_STRING(VIEW_BY_SERVICE,					"byService");
DECLARE_CONST_STRING(VIEW_BY_TARGET,					"byTarget");
DECLARE_CONST_STRING(STR_STAR,							"*");

bool CArcRouteUtil::m_bAEONRegistered = false;
DWORD CArcRouteUtil::ARC_HOST_SCHEMA = 0;
DWORD CArcRouteUtil::ARC_ROUTE_SCHEMA = 0;

bool CArcRouteUtil::Boot ()

//	Boot
//
//	Register shared AEON route types.

	{
	if (!m_bAEONRegistered)
		{
		TArray<IDatatype::SMemberDesc> HostSchema;
		HostSchema.Insert({ IDatatype::EMemberType::InstanceKeyVar, FIELD_ID, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_HOSTNAME, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_PROTOCOL, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_PORT, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_UNENCRYPTED_PORT, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_URL_PATH, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_ACTION, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_TARGET, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_OPTIONS, CAEONTypes::Get(IDatatype::ANY) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_SERVICE, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_PACKAGE, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_PRIORITY, CAEONTypes::Get(IDatatype::INT_32) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_STATUS, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_SOURCE, CAEONTypes::Get(IDatatype::STRING) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_CREATED_ON, CAEONTypes::Get(IDatatype::DATE_TIME) });
		HostSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_MODIFIED_ON, CAEONTypes::Get(IDatatype::DATE_TIME) });

		ARC_HOST_SCHEMA = CAEONTypes::RegisterSchema(TYPENAME_ARC_HOST_SCHEMA, HostSchema);

		TArray<IDatatype::SMemberDesc> RouteSchema;
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceKeyVar, FIELD_ROUTE_ID, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_ACTION, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_TARGET, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_OPTIONS, CAEONTypes::Get(IDatatype::ANY) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_PACKAGE, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_SERVICE, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_HOSTNAME, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_PROTOCOL, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_PORT, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_UNENCRYPTED_PORT, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_URL_PATH, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_PRIORITY, CAEONTypes::Get(IDatatype::INT_32) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_STATUS, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_SOURCE, CAEONTypes::Get(IDatatype::STRING) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_CREATED_ON, CAEONTypes::Get(IDatatype::DATE_TIME) });
		RouteSchema.Insert({ IDatatype::EMemberType::InstanceVar, FIELD_MODIFIED_ON, CAEONTypes::Get(IDatatype::DATE_TIME) });

		ARC_ROUTE_SCHEMA = CAEONTypes::RegisterSchema(TYPENAME_ARC_ROUTE_SCHEMA, RouteSchema);

		m_bAEONRegistered = true;
		}

	return true;
	}

CDatum CArcRouteUtil::CreateRouteTable (void)

//	CreateRouteTable
//
//	Creates an empty typed route table.

	{
	Boot();
	return CDatum::CreateTable(CAEONTypes::Get(ARC_ROUTE_SCHEMA));
	}

CDatum CArcRouteUtil::CreateHostTableDesc (void)

//	CreateHostTableDesc
//
//	Returns the Aeon table descriptor for Arc.hosts.

	{
	CDatum dDesc(CDatum::typeStruct);
	dDesc.SetElement(FIELD_NAME, TABLE_ARC_HOSTS);

	CDatum dX(CDatum::typeStruct);
	dX.SetElement(FIELD_KEY_TYPE, KEY_TYPE_UTF8);
	dDesc.SetElement(FIELD_X, dX);

	CDatum dColumns(CDatum::typeArray);
	dColumns.Append(STR_STAR);
	dColumns.Append(FIELD_PRIMARY_KEY);

	CDatum dByPackage(CDatum::typeStruct);
	CDatum dByPackageX(CDatum::typeStruct);
	dByPackageX.SetElement(FIELD_KEY, FIELD_PACKAGE);
	dByPackageX.SetElement(FIELD_KEY_TYPE, KEY_TYPE_UTF8);
	dByPackage.SetElement(FIELD_NAME, VIEW_BY_PACKAGE);
	dByPackage.SetElement(FIELD_X, dByPackageX);
	dByPackage.SetElement(FIELD_COLUMNS, dColumns);

	CDatum dByService(CDatum::typeStruct);
	CDatum dByServiceX(CDatum::typeStruct);
	dByServiceX.SetElement(FIELD_KEY, FIELD_SERVICE);
	dByServiceX.SetElement(FIELD_KEY_TYPE, KEY_TYPE_UTF8);
	dByService.SetElement(FIELD_NAME, VIEW_BY_SERVICE);
	dByService.SetElement(FIELD_X, dByServiceX);
	dByService.SetElement(FIELD_COLUMNS, dColumns);

	CDatum dByHost(CDatum::typeStruct);
	CDatum dByHostX(CDatum::typeStruct);
	dByHostX.SetElement(FIELD_KEY, FIELD_HOSTNAME);
	dByHostX.SetElement(FIELD_KEY_TYPE, KEY_TYPE_UTF8);
	dByHost.SetElement(FIELD_NAME, VIEW_BY_HOST);
	dByHost.SetElement(FIELD_X, dByHostX);
	dByHost.SetElement(FIELD_COLUMNS, dColumns);

	CDatum dByAction(CDatum::typeStruct);
	CDatum dByActionX(CDatum::typeStruct);
	dByActionX.SetElement(FIELD_KEY, FIELD_ACTION);
	dByActionX.SetElement(FIELD_KEY_TYPE, KEY_TYPE_UTF8);
	dByAction.SetElement(FIELD_NAME, VIEW_BY_ACTION);
	dByAction.SetElement(FIELD_X, dByActionX);
	dByAction.SetElement(FIELD_COLUMNS, dColumns);

	CDatum dByTarget(CDatum::typeStruct);
	CDatum dByTargetX(CDatum::typeStruct);
	dByTargetX.SetElement(FIELD_KEY, FIELD_TARGET);
	dByTargetX.SetElement(FIELD_KEY_TYPE, KEY_TYPE_UTF8);
	dByTarget.SetElement(FIELD_NAME, VIEW_BY_TARGET);
	dByTarget.SetElement(FIELD_X, dByTargetX);
	dByTarget.SetElement(FIELD_COLUMNS, dColumns);

	CDatum dSecondaryViews(CDatum::typeArray);
	dSecondaryViews.Append(dByPackage);
	dSecondaryViews.Append(dByService);
	dSecondaryViews.Append(dByHost);
	dSecondaryViews.Append(dByAction);
	dSecondaryViews.Append(dByTarget);
	dDesc.SetElement(FIELD_SECONDARY_VIEWS, dSecondaryViews);

	return dDesc;
	}
