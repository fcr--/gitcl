#!/bin/sh
#\
exec tclsh "$0" "$@"

proc test {name code} {
  puts -nonewline "test $name: "
  flush stdout
  uplevel $code
  puts ok
}

proc assert {args} {
  if {[llength $args] == 1} {
    set desc "assertion"
    set code [lindex $args 0]
  } elseif {[llength $args] == 2} {
    set desc "assertion ([lindex $args 0])"
    set code [lindex $args 1]
  } else {
    error "wrong # args: should be \"assert ?desc? expr\""
  }
  if {![uplevel [list expr $code]]} {
    error "$desc [list $code] failed"
  }
}

# MAIN TEST PROCEDURE:

test "loading library" {
  load ./libgitcl.so
}

test "no dependency is loaded at load time" {
  assert {[llength [gitcl::meta namespaces]] == 0}
}

test "require Gtk" {
  gitcl::require Gtk
}
test "Gtk is loaded" {
  assert {"Gtk" in [gitcl::meta namespaces]}
}
test "GObject is loaded as well" {
  assert {"GObject" in [gitcl::meta namespaces]}
}
test "nothing breaks if we require Gtk again" {
  set namespaces [gitcl::meta namespaces]
  gitcl::require Gtk
  assert {[lsort $namespaces] == [lsort [gitcl::meta namespaces]]}
}
test "nothing breaks if we require an already loaded dependency (GObject)" {
  set namespaces [gitcl::meta namespaces]
  gitcl::require GObject
  assert {[lsort $namespaces] == [lsort [gitcl::meta namespaces]]}
}
test "meta info pelota fails" {
  assert {[catch {gitcl::meta info pelota} res]}
  assert {[string match {*pelota*not loaded*} $res]}
}
test "some basic stuff included in Gtk" {
  set symbols [gitcl::meta info Gtk]
  foreach symbol {Button Widget Window init main main_quit} {
    assert "$symbol" {$symbol in $symbols}
  }
}
test "types in Gtk included stuff match" {
  foreach {symbol type} {
    main           function
    PAPER_NAME_A4  constant
    ToolbarStyle   enum
    TreeModel      interface
    KeySnoopFunc   callback
    AccelFlags     flags
    SymbolicColor  struct
    Frame          object
  } {
    assert "symbol=$symbol, type=$type" {
      $type == [dict get [gitcl::meta info Gtk $symbol] type]
    }
  }
}
test "some symbols in Gtk are deprecated" {
  set depr {}
  foreach name [gitcl::meta info Gtk] {
    dict incr depr [dict exists [gitcl::meta info Gtk $name] deprecated]
  }
  assert "$depr" {[dict get $depr 0] > 0 && [dict get $depr 1] > 0}
}
test "enums are registered and readable" {
  assert {"::gitcl::Gtk::ToolbarStyle" in [namespace children ::gitcl::Gtk]}
  assert {$::gitcl::Gtk::ToolbarStyle::both == 2}
}
