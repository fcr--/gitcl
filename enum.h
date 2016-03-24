#ifndef GITCL_ENUM_H
#define GITCL_ENUM_H 1

/**
 * Register a GObject enumeration in a Tcl namespace.
 *
 * @param interp Tcl interpreter
 * @param namespace Name of the namespace where the enum will be registered, i.e. ::gitcl::Gtk
 * @param info Representation of a GObject's enumeration.
 * @return TCL_OK or TCL_ERROR
 */
int gitcl_enum_register(Tcl_Interp * interp, const char * namespace, GIEnumInfo * info);

#endif
