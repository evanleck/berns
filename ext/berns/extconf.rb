# frozen_string_literal: true
require 'mkmf'

$CFLAGS = '-O3 -msse4' # rubocop:disable Style/GlobalVars

dir_config('berns')
create_header
create_makefile 'berns/berns'
