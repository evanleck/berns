# frozen_string_literal: true
require 'mkmf'

# -std=c99
# -std=c11
# hescape -O3 -msse4 -std=c99

$CFLAGS = '-O3 -msse4' # rubocop:disable Style/GlobalVars

dir_config('berns')

create_header
create_makefile 'berns/berns'
