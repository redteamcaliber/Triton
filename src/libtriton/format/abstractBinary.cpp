//! \file
/*
**  Copyright (C) - Triton
**
**  This program is under the terms of the BSD License.
*/

#include <cstdio>
#include <stdexcept>

#include <abstractBinary.hpp>
#include <elf.hpp>



namespace triton {
  namespace format {

    AbstractBinary::AbstractBinary() {
      this->format = triton::format::BINARY_INVALID;
      this->binary = nullptr;
    }


    AbstractBinary::AbstractBinary(const std::string& path) {
      this->format = triton::format::BINARY_INVALID;
      this->binary = nullptr;
      this->loadBinary(path);
    }


    AbstractBinary::~AbstractBinary() {
      delete this->binary;
    }


    triton::format::binary_e AbstractBinary::getFormat(void) const {
      return this->format;
    }


    void AbstractBinary::loadBinary(const std::string& path) {
      FILE* fd = nullptr;
      triton::uint8 raw[8] = {0};
      triton::usize size = 0;

      // Open the file
      fd = fopen(path.c_str(), "rb");
      if (fd == nullptr)
        throw std::runtime_error("AbstractBinary::loadBinary(): Cannot open the binary file.");

      // Get the binary size
      fseek(fd, 0, SEEK_END);
      size = ftell(fd);
      rewind(fd);

      if (size < sizeof(raw))
        throw std::runtime_error("AbstractBinary::loadBinary(): The binary file is too small.");

      // Read only the magic number
      if (fread(raw, 1, sizeof(raw), fd) != sizeof(raw))
        throw std::runtime_error("AbstractBinary::loadBinary(): Cannot read the file binary.");

      // Close the file
      fclose(fd);

      // Set the binary format according the magic number
      if ((*((triton::uint32*)(raw))) == triton::format::MAGIC_ELF)
        this->format = triton::format::BINARY_ELF;

      else if ((*((triton::uint32*)(raw))) == triton::format::MAGIC_MACHO32)
        this->format = triton::format::BINARY_MACHO;

      else if ((*((triton::uint32*)(raw))) == triton::format::MAGIC_MACHO64)
        this->format = triton::format::BINARY_MACHO;

      else if ((*((triton::uint16*)(raw))) == triton::format::MAGIC_PE)
        this->format = triton::format::BINARY_PE;

      else
        this->format = triton::format::BINARY_INVALID;

      // Parse the binary according to the format
      this->parseBinary(path);
    }


    void AbstractBinary::parseBinary(const std::string& path) {
      switch (this->format) {
        case triton::format::BINARY_ELF:
          delete this->binary;
          this->binary = new triton::format::elf::ELF(path);
          if (!this->binary)
            throw std::runtime_error("AbstractBinary::parseBinary(): Not enough memory.");
          break;

        // TODO
        case triton::format::BINARY_MACHO:
        case triton::format::BINARY_PE:
        default:
          throw std::runtime_error("AbstractBinary::parseBinary(): Unsupported binary format.");
      }
    }


    triton::format::BinaryInterface* AbstractBinary::getBinary(void) {
      return this->binary;
    }


    triton::format::elf::ELF* AbstractBinary::getElf(void) {
      if (this->format != triton::format::BINARY_ELF)
        throw std::runtime_error("AbstractBinary::getElf(): The abstract binary is not an ELF.");
      return reinterpret_cast<triton::format::elf::ELF*>(this->binary);
    }


    const std::string& AbstractBinary::getPath(void) const {
      if (!this->binary)
        throw std::runtime_error("AbstractBinary::getPath(): You must load the binary before.");
      return this->binary->getPath();
    }


    const std::list<triton::format::MemoryMapping>& AbstractBinary::getMemoryMapping(void) const {
      if (!this->binary)
        throw std::runtime_error("AbstractBinary::getMemoryMapping(): You must load the binary before.");
      return this->binary->getMemoryMapping();
    }

  }; /* format namespace */
}; /* triton namespace */
