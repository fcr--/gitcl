#include <girepository.h>
#include <stdlib.h>
#include <string.h>
#include "tcl.h"

static Tcl_Command require_cmd_ref = NULL,
		   meta_cmd_ref = NULL;

static Tcl_Obj *str_name, *str_invalid, *str_function, *str_callback,
	       *str_struct, *str_boxed, *str_enum, *str_flags, *str_object,
	       *str_interface, *str_constant, *str_union, *str_value,
	       *str_signal, *str_vfunc, *str_property, *str_field, *str_arg,
	       *str_type, *str_unresolved, *str_attributes;

int require_cmd(void * privdata, Tcl_Interp * interp, int argc, Tcl_Obj * const argv[]) {
  if (argc != 2 && argc != 3) {
    Tcl_WrongNumArgs(interp, 1, argv, "namespace ?version?");
    return TCL_ERROR;
  }
  const char * namespace = Tcl_GetString(argv[1]);
  const char * version = argc == 3 ? Tcl_GetString(argv[2]) : NULL;
  GError * error = NULL;
  GITypelib * typelib = g_irepository_require(NULL, namespace, version, 0, &error);
  if (!typelib) {
    Tcl_SetObjResult(interp, error ?
	Tcl_ObjPrintf("error on require \"%s\": [%d] %s\n",
	    namespace, error->code, error->message) :
	Tcl_ObjPrintf("unspecified error on require \"%s\"", namespace));
    return TCL_ERROR;
  }
  return TCL_OK;
}

static Tcl_Obj * get_metainfo(Tcl_Interp * interp, GIBaseInfo * info) {
  Tcl_Obj * res = Tcl_NewDictObj();
  // name:
  Tcl_DictObjPut(interp, res, str_name, Tcl_NewStringObj(g_base_info_get_name(info), -1));
  // type:
  Tcl_Obj *typestr;
  switch (g_base_info_get_type(info)) {
    case GI_INFO_TYPE_INVALID:
      typestr = str_invalid;
      break;
    case GI_INFO_TYPE_FUNCTION:
      typestr = str_function;
      break;
    case GI_INFO_TYPE_CALLBACK:
      typestr = str_callback;
      break;
    case GI_INFO_TYPE_STRUCT:
      typestr = str_struct;
      break;
    case GI_INFO_TYPE_BOXED:
      typestr = str_boxed;
      break;
    case GI_INFO_TYPE_ENUM:
      typestr = str_enum;
      break;
    case GI_INFO_TYPE_FLAGS:
      typestr = str_flags;
      break;
    case GI_INFO_TYPE_OBJECT:
      typestr = str_object;
      break;
    case GI_INFO_TYPE_INTERFACE:
      typestr = str_interface;
      break;
    case GI_INFO_TYPE_CONSTANT:
      typestr = str_constant;
      break;
    case GI_INFO_TYPE_UNION:
      typestr = str_union;
      break;
    case GI_INFO_TYPE_VALUE:
      typestr = str_value;
      break;
    case GI_INFO_TYPE_SIGNAL:
      typestr = str_signal;
      break;
    case GI_INFO_TYPE_VFUNC:
      typestr = str_vfunc;
      break;
    case GI_INFO_TYPE_PROPERTY:
      typestr = str_property;
      break;
    case GI_INFO_TYPE_FIELD:
      typestr = str_field;
      break;
    case GI_INFO_TYPE_ARG:
      typestr = str_arg;
      break;
    case GI_INFO_TYPE_TYPE:
      typestr = str_type;
      break;
    case GI_INFO_TYPE_UNRESOLVED:
      typestr = str_unresolved;
      break;
    default:
      typestr = str_invalid;
      break;
  }
  Tcl_DictObjPut(interp, res, str_type, typestr);
  // attributes:
  {
    Tcl_Obj *attributes = Tcl_NewDictObj();
    GIAttributeIter iter = { 0, };
    char *name;
    char *value;
    while (g_base_info_iterate_attributes (info, &iter, &name, &value)) {
      Tcl_DictObjPut(interp, attributes, Tcl_NewStringObj(name, -1), Tcl_NewStringObj(value, -1));
    }
    Tcl_DictObjPut(interp, res, str_attributes, attributes);
  }
  return res;
}

static int meta_cmd(void * privdata, Tcl_Interp * interp, int argc, Tcl_Obj * const argv[]) {
  if (argc < 2) {
    Tcl_WrongNumArgs(interp, 1, argv, "subcommand ...");
    return TCL_ERROR;
  }
  if (!strcmp(Tcl_GetString(argv[1]), "info")) {
    if (argc != 3) {
      Tcl_WrongNumArgs(interp, 2, argv, "namespace");
      return TCL_ERROR;
    }
    const char * namespace = Tcl_GetString(argv[2]);
    if (!strcmp("----", namespace)) {
      // TODO: return something with options! for tclreadline support
      Tcl_FreeResult(interp);
      return TCL_OK;
    }
    if (!g_irepository_is_registered(NULL, namespace, NULL)) {
      Tcl_SetObjResult(interp, Tcl_ObjPrintf(
	    "namespace \"%s\" is not loaded",
	    namespace));
      return TCL_ERROR;
    }
    printf("namespace: <%s>\n", namespace);
    int n = g_irepository_get_n_infos(NULL, namespace);
    if (n < 0) {
      Tcl_SetResult(interp, "g_irepository_get_n_infos < 0.", TCL_STATIC);
      return TCL_ERROR;
    }
    Tcl_Obj * res = Tcl_NewListObj(0, NULL);
    for (int i = 0; i < n; i++) {
      GIBaseInfo * info = g_irepository_get_info(NULL, namespace, i);
      const char * name = g_base_info_get_name(info);
      Tcl_ListObjAppendElement(interp, res, Tcl_NewStringObj(name, -1));
      g_base_info_unref(info);
    }
    Tcl_SetObjResult(interp, res);
    return TCL_OK;
  } else if (!strcmp(Tcl_GetString(argv[1]), "namespaces")) {
    if (argc != 2) {
      Tcl_WrongNumArgs(interp, 2, argv, NULL);
      return TCL_ERROR;
    }
    Tcl_Obj * res = Tcl_NewListObj(0, NULL);
    gchar ** list = g_irepository_get_loaded_namespaces(NULL);
    gchar ** cursor = list;
    while (*cursor) {
      Tcl_ListObjAppendElement(interp, res, Tcl_NewStringObj(*cursor, -1));
      free(*cursor);
      cursor++;
    }
    free(list);
    Tcl_SetObjResult(interp, res);
    return TCL_OK;
  } else {
    Tcl_SetObjResult(interp, Tcl_ObjPrintf(
	  "unknown or ambiguous subcommand \"%s\": must be info or namespaces",
	  Tcl_GetString(argv[1])));
    return TCL_ERROR;
  }
  return TCL_OK;
}

#define INIT_TCL_STRING(name) str_##name = Tcl_NewStringObj(#name, -1)

int Gitcl_Init(Tcl_Interp * interp) {
  if (!Tcl_CreateNamespace(interp, "gitcl", NULL, NULL)) return TCL_ERROR;
  INIT_TCL_STRING(name);
  INIT_TCL_STRING(invalid);
  INIT_TCL_STRING(function);
  INIT_TCL_STRING(callback);
  INIT_TCL_STRING(struct);
  INIT_TCL_STRING(boxed);
  INIT_TCL_STRING(enum);
  INIT_TCL_STRING(flags);
  INIT_TCL_STRING(object);
  INIT_TCL_STRING(interface);
  INIT_TCL_STRING(constant);
  INIT_TCL_STRING(union);
  INIT_TCL_STRING(value);
  INIT_TCL_STRING(signal);
  INIT_TCL_STRING(vfunc);
  INIT_TCL_STRING(property);
  INIT_TCL_STRING(field);
  INIT_TCL_STRING(arg);
  INIT_TCL_STRING(type);
  INIT_TCL_STRING(unresolved);
  require_cmd_ref = Tcl_CreateObjCommand(interp, "::gitcl::require", &require_cmd, NULL, NULL);
  if (!require_cmd_ref) return TCL_ERROR;
  meta_cmd_ref = Tcl_CreateObjCommand(interp, "::gitcl::meta", &meta_cmd, NULL, NULL);
  if (!meta_cmd_ref) return TCL_ERROR;
  return TCL_OK;
}