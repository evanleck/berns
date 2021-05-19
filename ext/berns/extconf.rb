# frozen_string_literal: true
require 'mkmf'

dir_config 'berns'

append_cflags '-O3'
append_cflags '-Wshadow'
append_cflags '-Wstrict-overflow'
append_cflags '-flto'
append_cflags '-fno-strict-aliasing'
append_cflags '-msse4'
append_cflags '-std=c99'

create_makefile 'berns/berns'
