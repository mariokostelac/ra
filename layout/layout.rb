#!/usr/bin/env ruby
require 'optparse'
require 'Date'
require 'FileUtils'

$start_time = DateTime.now

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

    before = Time.now
    res = @cmd.call
    after = Time.now

    puts Colors::green
    passed = human_time((after - before) * 1000)
    puts "Finished in #{passed}"
    puts '=' * 80

    @@task_number += 1

    res
  end

end

def human_time(milisecs)
  units = ['ms', 1000, 's', 60, 'm', 60]

  parts = []

  left = milisecs.to_i
  i = 0

  while i < units.size do
    unit_label = units[i]
    till_next = units[i + 1]

    in_this_unit = left % till_next
    left -= in_this_unit
    left /= till_next

    parts.push(unit_label)
    parts.push(in_this_unit.to_s) 

    break if left == 0

    i += 2
  end

  parts.push('h', left) if left > 0

  parts.reverse.take(6).join('')
end

def help
  puts $options_parser
  exit 1
end

def parse_options!
  OptionParser.new do |opts|
    $options_parser = opts
    opts.banner = "Usage: #{$0} <fasta_reads> <mhap_overlaps>"

    opts.on("-s", "--settings settings_file", "Set given settings file") do |settings_file|
      $options[:settings_file] = settings_file
    end

    opts.on("-d", "--directory dirpath", "Set given dirpath as working directory") do |dir|
      working_dir = dir
    end

    opts.on("-f", "--use_smart_filter", "Uses 'smart' overlaps filter that cuts right tail of gauss distribution") do |dir|
      $options[:use_smart_filter] = true
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

def draw_graph_bin(debug: false)
  File.join(bin_dir(debug: debug), "overlap2dot")
end

def filter_bad_overlaps_bin(debug: false)
  File.join(bin_dir(debug: debug), "filter_erroneous_overlaps")
end

def depot_bin(debug: false)
  File.join(bin_dir(debug: debug), "depot")
end

def run_filter_contained(overlaps_filename)
  working_directory = working_dir
  cmd = "#{filter_contained_bin} -D #{depot_path} -x #{overlaps_filename} -d #{working_directory}"
  puts(cmd)
  system(cmd)
end

def run_dovetail_overlaps(overlaps_filename)
  working_directory = working_dir
  cmd = "#{create_dovetail_bin} -D #{depot_path} -x #{overlaps_filename} -d #{working_directory}"
  puts(cmd)
  system(cmd)
end

def run_filer_transitive(overlaps_filename)
  working_directory = working_dir
  cmd = "#{filter_transitives_bin} -D #{depot_path} -x #{overlaps_filename} -d #{working_directory}"
  puts(cmd)
  system(cmd)
end

def run_unitigger(overlaps_filename)
  settings = ""
  settings = "-b #{settings_file}" if settings_file.to_s.length > 0

  working_directory = working_dir
  cmd = "#{unitigger_bin} -D #{depot_path} -x #{overlaps_filename} -d #{working_directory} #{settings}"
  puts(cmd)
  system(cmd)
end

def run_graphviz(overlaps_filename)
  working_directory = working_dir
  graph_filename = "#{working_directory}/genome.svg"
  cmd = "#{draw_graph_bin} < #{overlaps_filename} | neato -T svg -o #{graph_filename}"
  puts(cmd)
  system(cmd)
end

def run_filter_bad_overlaps(overlaps_filename)
  working_directory = working_dir
  output = File.join(working_directory, "overlaps.filtered")
  cmd = "#{filter_bad_overlaps_bin} -x #{overlaps_filename} > #{output}"
  puts(cmd)
  system(cmd)

  output
end

def run_import_reads(depot_path, reads_filename)
  cmd = "#{depot_bin} -r #{reads_filename} -d #{depot_path} import_reads"
  puts(cmd)
  system(cmd)
end

def ensure_dir(dirpath)
  FileUtils::mkdir_p dirpath
end

$options_parser = nil

$options = {}

def working_dir
  if $options.has_key?(:working_dir)
    return $options[:working_dir]
  end

  suffix = $start_time.strftime("%Y%m%d_%H%M%S")
  working_dir = "layout_#{suffix}"

  working_dir
end

def working_dir=(str)
  $options[:working_dir] = str
end

def depot_path=(str)
  $options[:depot_path] = str
end

def depot_path
  if $options.has_key?(:depot_path)
    return $options[:depot_path]
  end

  suffix = $start_time.strftime("%Y%m%d_%H%M%S")
  depot_path = "layout_#{suffix}_depot"

  depot_path
end

def use_smart_filter?
  if $options.has_key?(:use_smart_filter)
    return $options[:use_smart_filter]
  end

  false
end

def settings_file
  return $options[:settings_file]
end

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
  puts "Filter erroneous overlaps binary: #{filter_bad_overlaps_bin}"
  puts
  puts "Reads filename: #{reads_filename}"
  puts "Overlaps filename: #{overlaps_filename}"
  puts "Assembly directory: #{working_dir}"
  puts "Depot path: #{depot_path}"
  puts

  Task.new "PREPARING ASSEMBLY DIRECTORY" do
    if !ensure_dir(working_dir)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end
  end.run

  Task.new "FILLING DEPOT WITH READS" do
    if !run_import_reads(depot_path, reads_filename)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end

    File.join(working_dir, "overlaps.nocont")
  end.run

  filter_contained = Task.new "FILTERING CONTAINED READS" do
    if !run_filter_contained(overlaps_filename)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end

    File.join(working_dir, "overlaps.nocont")
  end
  overlaps_filename = filter_contained.run

  create_dovetail = Task.new "CREATING DOVETAIL OVERLAPS" do
    if !run_dovetail_overlaps(overlaps_filename)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end

    File.join(working_dir, "overlaps.dovetail")
  end
  overlaps_filename = create_dovetail.run

  if use_smart_filter?
    filter_bad_overlaps = Task.new "FILTERING BAD OVERLAPS" do
      run_filter_bad_overlaps(overlaps_filename)
    end
    overlaps_filename = filter_bad_overlaps.run
  end

  filter_transitive = Task.new "FILTERING TRANSITIVE OVERLAPS" do
    if !run_filer_transitive(overlaps_filename)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end

    File.join(working_dir, "overlaps.notran")
  end
  overlaps_filename = filter_transitive.run

  find_unitigs = Task.new "FINDING UNITIGS" do
    if !run_unitigger(overlaps_filename)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end
  end
  find_unitigs.run

  Task.new "DRAWING GRAPHS" do
    if !run_graphviz(overlaps_filename)
      puts 'Process exited with non-zero exit status, stopping here!'
      exit 1
    end
  end.run
end

main()
