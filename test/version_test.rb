# frozen_string_literal: true
require 'berns'
require 'bigdecimal'
require 'minitest/autorun'

describe Berns::VERSION do
  specify 'it has a version' do
    refute_nil Berns::VERSION
  end
end
