# frozen_string_literal: true
require 'bundler/gem_tasks'
require 'rake/clean'
require 'rake/extensiontask'
require 'rake/testtask'
require 'rubocop/rake_task'

CLEAN.add('pkg', 'tmp', 'lib/berns/berns.so', 'lib/berns/berns.bundle')

Rake::ExtensionTask.new('berns') do |ext|
  ext.lib_dir = 'lib/berns'
end

Rake::TestTask.new(:test) do |t|
  t.deps = %i[clean compile]
  t.libs << 'lib'
  t.test_files = FileList['test/**/*_test.rb']
end

RuboCop::RakeTask.new

task default: %i[test rubocop]
