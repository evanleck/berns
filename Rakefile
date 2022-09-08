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

RuboCop::RakeTask.new

Rake::TestTask.new(:test) do |task|
  task.libs << 'lib'
  task.test_files = FileList['test/**/*_test.rb']
end

desc 'Clean, compile, test, and lint.'
task suite: %i[clean compile test rubocop]

task default: %i[test rubocop]
