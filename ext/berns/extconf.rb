# frozen_string_literal: true
require 'mkmf'

$CFLAGS << ' -O2 -msse4 -std=c99' # rubocop:disable Style/GlobalVars

dir_config('berns')
create_header
create_makefile 'berns/berns'
