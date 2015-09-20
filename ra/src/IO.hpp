/*!
 * @file IO.hpp
 *
 * @brief Input Output methods header file
 *
 * @author: rvaser
 * @date Apr 21, 2015
 */

#pragma once

#include "Contig.hpp"
#include "Read.hpp"
#include "Overlap.hpp"
#include "StringGraph.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief Method for Read input
 * @details Method reads from file in FASTA format and creates
 * Read objects
 *
 * @param [out] reads vector of Read objects pointers
 * @param [in] path path to file where the Read objects are stored
 */
void readFastaReads(std::vector<Read*>& reads, const char* path);

/*!
 * @brief Method for Read input
 * @details Method reads from file in FASTQ format and creates
 * Read objects
 *
 * @param [out] reads vector of Read objects pointers
 * @param [in] path path to file where the Read objects are stored
 */
void readFastqReads(std::vector<Read*>& reads, const char* path);

/*!
 * @brief Method for Read input
 * @details Method reads from file in AFG format and creates
 * Read objects
 *
 * @param [out] reads vector of Read objects pointers
 * @param [in] path path to file where the Read objects are stored
 */
void readAfgReads(std::vector<Read*>& reads, const char* path);

/*!
 * @brief Method for Read input
 * @details Method reads from stream in AFG format and creates
 * Read objects
 *
 * @param [out] reads vector of Read objects pointers
 * @param [in] input stream
 */
void readAfgReads(std::vector<Read*>& reads, std::istream& input);

/*!
 * @brief Method for Read output
 * @details Method writes Read objects to file in FASTA format
 *
 * @param [in] reads vector of Read objects pointers
 * @param [in] path path to file where the Read objects will be stored
 * (if null, stdout is used)
 */
void writeFastaReads(const std::vector<Read*>& reads, const char* path);

/*!
 * @brief Method for Read output
 * @details Method writes Read objects to file in AFG format
 *
 * @param [in] reads vector of Read objects pointers
 * @param [in] path path to file where the Read objects will be store
 * (if null, stdout is used)
 */
void writeAfgReads(const std::vector<Read*>& reads, const char* path);

/*!
 * @brief Method for Overlap input
 * @details Method reads from file in AFG format and creates
 * Overlap objects
 *
 * @param [out] overlaps vector of Overlap objects pointers
 * @param [in] path path to file where the Overlap objects are stored
 */
void readAfgOverlaps(std::vector<DovetailOverlap*>& overlaps, const char* path);

/*!
 * @brief Method for Overlap input
 * @details Method reads from file in AFG format and creates
 * Overlap objects
 *
 * @param [out] overlaps vector of Overlap objects pointers
 * @param [in] input stream where Overlap objects flow through
 */
void readAfgOverlaps(std::vector<DovetailOverlap*>& overlaps, std::istream& input);

/*!
 * @brief Method for Overlap output
 * @details Method writes Overlap objects to file
 *
 * @param [in] reads vector of Overlap objects pointers
 * @param [in] path path to file where the Overlap objects will be stored
 * (if null, stdout is used)
 */
void write_overlaps(const std::vector<Overlap*>& overlaps, const char* path);

/*!
 * @brief Method for Overlap output
 * @details Method writes Overlap objects to file
 *
 * @param [in] reads vector of Overlap objects pointers
 * @param [in] path path to file where the Overlap objects will be stored
 * (if null, stdout is used)
 */
void write_overlaps(const std::vector<Overlap*>& overlaps, const std::string path);

/*!
 * @brief Method for reading dovetail overlaps
 * @details Method reads DovetailOverlaps from file
 *
 * @param [in] overlaps vector of Overlap objects pointers
 * @param [in] fd file descriptor
 */
void read_dovetail_overlaps(std::vector<DovetailOverlap*>* overlaps, FILE* fd);

/*!
 * @brief Method for Contig input
 * @details Method reads from file in AFG format and creates
 * Contig objects
 *
 * @param [out] contigs vector of Contig objects pointers
 * @param [in] path path to file where the Contig objects are stored
 */
void readAfgContigs(std::vector<Contig*>& contigs, const char* path);

/*!
 * @brief Method for Contig output
 * @details Method writes Contig objects to file in AFG format
 *
 * @param [in] reads vector of Contig objects pointers
 * @param [in] path path to file where the Contig objects will be stored
 * (if null, stdout is used)
 */
void writeAfgContigs(const std::vector<Contig*>& contigs, const char* path);

/*!
 * @brief Method check if the given file exists
 *
 * @param [in] path path to file
 * @return true if the file exists
 */
bool fileExists(const char* path);

/*!
 * @brief Method for file input
 * @details Method reads bytes from file and stores them in byte buffer
 *
 * @param [out] bytes byte buffer
 * @param [in] path path to file
 */
void fileRead(char** bytes, const char* path);

/*!
 * @brief Method for file output
 * @details Method writes bytes from byte buffer to file
 *
 * @param [in] bytes byte buffer
 * @param [in] bytesLen byte buffer length
 * @param [in] path path to file
 */
void fileWrite(const char* bytes, size_t bytesLen, const char* path);
