#define _GNU_SOURCE

#include <girepository.h>
#include <stdlib.h>
#include <string.h>
#include "tcl.h"

int gitcl_enum_register(Tcl_Interp * interp, const char * namespace, GIEnumInfo * info) {
  if (!GI_IS_ENUM_INFO(info)) {
    Tcl_SetResult(interp, "called gitcl_enum_register with a non enum info", TCL_STATIC);
    return TCL_ERROR;
  }
  const char * name = g_base_info_get_name(info);
  if (!name) {
    Tcl_SetResult(interp, "gitcl_enum_register: enum lacks a name", TCL_STATIC);
    return TCL_ERROR;
  }
  char * enum_ns = NULL;
  asprintf(&enum_ns, "%s::%s", namespace, name);
  if (!enum_ns) {
    Tcl_SetResult(interp, "gitcl_enum_register: internal error on asprintf", TCL_STATIC);
    return TCL_ERROR;
  }
  Tcl_Namespace * enum_ns_ptr = Tcl_CreateNamespace(interp, enum_ns, NULL, NULL);
  if (!enum_ns_ptr) {
    Tcl_SetResult(interp, "gitcl_enum_register: sub namespace for symbol already created", TCL_STATIC);
    goto error_free;
  }
  int n = g_enum_info_get_n_values(info);
  if (n < 0) {
    Tcl_SetResult(interp, "gitcl_enum_register: g_enum_info_get_n_values gobject is crazy!?", TCL_STATIC);
    goto error_delete_ns;
  }
  for (int i = 0; i < n; i++) {
    GIValueInfo * value = g_enum_info_get_value(info, i);
    if (!value) {
      Tcl_SetResult(interp, "gitcl_enum_register: internal error at g_enum_info_get_value", TCL_STATIC);
      goto error_delete_ns;
    }
    const char * value_name = g_base_info_get_name(value);
    if (!value_name) {
      Tcl_SetResult(interp, "gitcl_enum_register: enum value lacks a name", TCL_STATIC);
      goto error_delete_ns;
    }
    // tcl docs say Tcl_WideInt is at least 64-bits wide...
    Tcl_WideInt value_num = g_value_info_get_value(value);
    Tcl_ObjSetVar2(interp, Tcl_ObjPrintf("%s::%s", enum_ns, value_name), NULL,
	Tcl_NewWideIntObj(value_num), TCL_GLOBAL_ONLY);
  }
  free(enum_ns);
  return TCL_OK;
error_delete_ns:
  Tcl_DeleteNamespace(enum_ns_ptr);
error_free:
  free(enum_ns);
  return TCL_ERROR;
}
