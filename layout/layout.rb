#!/usr/bin/env ruby
require 'optparse'
require 'Date'
require 'FileUtils'

module Colors
  def self.black;          "\e[30m" end
  def self.red;            "\e[31m" end
  def self.green;          "\e[32m" end
  def self.brown;          "\e[33m" end
  def self.blue;           "\e[34m" end
  def self.magenta;        "\e[35m" end
  def self.cyan;           "\e[36m" end
  def self.gray;           "\e[37m" end

  def self.end;            "\e[0m" end
end

class Task

  @@task_number = 1

  def initialize(name, &block)
    @name = name
    @cmd = block
  end

  def run
    puts Colors::green
    puts "#{@@task_number}. #{@name}"
    puts '=' * 80

    puts Colors::cyan
    res = @cmd.call

    puts Colors::green
    puts '=' * 80

    @@task_number += 1

    res
  end

end

def help
  puts $options_parser
  exit 1
end

def parse_options!
  OptionParser.new do |opts|
    $options_parser = opts
    opts.banner = "Usage: #{$0} <fasta_reads> <mhap_overlaps>"

    opts.on("-d", "--directory dirpath", "Set given dirpath as working directory") do |dir|
      $options[:working_dir] = dir
    end

    opts.on_tail("-h", "--help", "Show this message") do
      help
    end

  end.parse!
end

def parse_arguments
  if ARGV.length < 2
    help
  end

  ARGV.take 2
end

def working_dir
  suffix = DateTime.now.strftime("%Y%m%d_%H%M%S")
  "layout_#{suffix}"
end

def bin_dir(debug: false)
  parts = []
  parts.push __dir__
  parts.push "debug" if debug == true
  File.join(parts)
end

def filter_contained_bin(debug: false)
  File.join(bin_dir(debug: debug), "filter_contained")
end

def create_dovetail_bin(debug: false)
  File.join(bin_dir(debug: debug), "widen_overlaps")
end

def filter_transitives_bin(debug: false)
  File.join(bin_dir(debug: debug), "filter_transitive")
end

def unitigger_bin(debug: false)
  File.join(bin_dir(debug: debug), "unitigger")
end

def run_filter_contained(reads_filename, overlaps_filename)
  working_directory = $options[:working_dir]
  cmd = "#{filter_contained_bin} -r #{reads_filename} -x #{overlaps_filename} -d #{working_directory}"
  puts(cmd)
  system(cmd)
end

def run_dovetail_overlaps(reads_filename, overlaps_filename)
  working_directory = $options[:working_dir]
  cmd = "#{create_dovetail_bin} -r #{reads_filename} -x #{overlaps_filename} -d #{working_directory}"
  puts(cmd)
  system(cmd)
end

def run_filer_transitive(reads_filename, overlaps_filename)
  working_directory = $options[:working_dir]
  cmd = "#{filter_transitives_bin} -r #{reads_filename} -x #{overlaps_filename} -d #{working_directory}"
  puts(cmd)
  system(cmd)
end

def run_unitigger(reads_filename, overlaps_filename)
  working_directory = $options[:working_dir]
  cmd = "#{unitigger_bin} -r #{reads_filename} -x #{overlaps_filename} -d #{working_directory}"
  puts(cmd)
  system(cmd)
end

def ensure_dir(dirpath)
  FileUtils::mkdir_p dirpath
end

def line
  "=" * 80
end

$options_parser = nil

$options = {
  :working_dir => working_dir
}

def main
  parse_options!
  reads_filename, overlaps_filename = parse_arguments

  puts Colors::green

  puts 'LAYOUT phase'
  puts
  puts "Filter containments binary: #{filter_contained_bin}"
  puts "Create dovetail overlaps binary: #{create_dovetail_bin}"
  puts "Filter transitive overlaps binary: #{filter_transitives_bin}"
  puts "Unitigger binary: #{unitigger_bin}"
  puts
  puts "Reads filename: #{reads_filename}"
  puts "Overlaps filename: #{overlaps_filename}"
  puts "Assembly directory: #{$options[:working_dir]}"
  puts

  step = 1

  Task.new "PREPARING ASSEMBLY DIRECTORY" do
    if !ensure_dir($options[:working_dir])
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end
  end.run

  filter_contained = Task.new "FILTERING CONTAINED READS" do
    if !run_filter_contained(reads_filename, overlaps_filename)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end

    File.join($options[:working_dir], "overlaps.nocont")
  end
  overlaps_filename = filter_contained.run

  create_dovetail = Task.new "CREATING DOVETAIL OVERLAPS" do
    if !run_dovetail_overlaps(reads_filename, overlaps_filename)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end

    File.join($options[:working_dir], "overlaps.dovetail")
  end
  overlaps_filename = create_dovetail.run

  filter_transitive = Task.new "FILTERING TRANSITIVE OVERLAPS" do
    if !run_filer_transitive(reads_filename, overlaps_filename)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end

    File.join($options[:working_dir], "overlaps.notran")
  end
  overlaps_filename = filter_transitive.run

  find_unitigs = Task.new "FINDING UNITIGS" do
    if !run_unitigger(reads_filename, overlaps_filename)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end
  end
  find_unitigs.run

end

main
