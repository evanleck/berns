# frozen_string_literal: true
require 'mkmf'

dir_config('berns')

create_header
create_makefile 'berns/berns'
