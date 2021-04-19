#pragma once

#include <cassert>
#include <cstdint>

#include "elf.hpp"

/*! \brief Calculate relocation */
template<typename RELOC>
struct Relocator {
	/*! \brief Offset in object to be relocated */
	const RELOC & entry;

	/*! \brief Constructor */
	Relocator(const RELOC & entry) : entry(entry) {
		assert(entry.valid());
	}

	/*! \brief Get relocation value (for external symbol)
	 * \param base Base address in target memory of the object to which this relocation belongs to
	 * \param symbol (External) Symbol (from another object)
	 * \param symbol_base base address of the object providing the external symbol
	 * \param global_offset_table address of the global offset table (in this object)
	 * \param plt_entry PLT entry of the symbol
	 */
	uintptr_t value(uintptr_t base, const ELF_Def::Symbol & symbol, uintptr_t symbol_base, uintptr_t global_offset_table = 0, uintptr_t plt_entry = 0) const {
		assert(symbol.valid());

		const intptr_t A = entry.addend();
		const uintptr_t B = base;
		const uintptr_t G = symbol_base + symbol.value();
		const uintptr_t GOT = global_offset_table;
		const uintptr_t L = plt_entry;
		const uintptr_t P = base + entry.offset();
		const uintptr_t S = symbol_base + symbol.value();
		const uintptr_t Z = symbol.size();

		switch (entry.elf().header.machine()) {
			case ELF_Def::EM_386:
			case ELF_Def::EM_486:
			switch (entry.type()) {
				case ELF_Def::R_386_NONE:
					return 0;

				case ELF_Def::R_386_COPY:
					assert(false);
					return 0;

				case ELF_Def::R_386_8:
				case ELF_Def::R_386_16:
				case ELF_Def::R_386_32:
					return S + A;

				case ELF_Def::R_386_PC8:
				case ELF_Def::R_386_PC16:
				case ELF_Def::R_386_PC32:
					return S + A - P;

				case ELF_Def::R_386_GOT32:
					return G + A;

				case ELF_Def::R_386_PLT32:
					return L + A - P;

				case ELF_Def::R_386_GLOB_DAT:
				case ELF_Def::R_386_JMP_SLOT:
					return S;

				case ELF_Def::R_386_RELATIVE:
					return B + A;

				case ELF_Def::R_386_GOTOFF:
					return S + A - GOT;

				case ELF_Def::R_386_GOTPC:
					return GOT + A - P;

				case ELF_Def::R_386_32PLT:
					return L + A;

				case ELF_Def::R_386_SIZE32:
					return Z + A;

				default: // Not recognized!
					assert(false);
					return 0;
			}

			case ELF_Def::EM_X86_64:
				switch (entry.type()) {
					case ELF_Def::R_X86_64_NONE:
						return 0;

					case ELF_Def::R_X86_64_COPY:
						assert(false);
						return 0;

					case ELF_Def::R_X86_64_GLOB_DAT:
					case ELF_Def::R_X86_64_JUMP_SLOT:
						return S;

					case ELF_Def::R_X86_64_8:
					case ELF_Def::R_X86_64_16:
					case ELF_Def::R_X86_64_32:
					case ELF_Def::R_X86_64_32S:
					case ELF_Def::R_X86_64_64:
						return S + A;

					case ELF_Def::R_X86_64_PC8:
					case ELF_Def::R_X86_64_PC16:
					case ELF_Def::R_X86_64_PC32:
					case ELF_Def::R_X86_64_PC64:
						return S + A - P;

					case ELF_Def::R_X86_64_GOT32:
						return G + A;

					case ELF_Def::R_X86_64_PLT32:
						return L + A - P;

					case ELF_Def::R_X86_64_RELATIVE:
					case ELF_Def::R_X86_64_RELATIVE64:
						return B + A;

					case ELF_Def::R_X86_64_GOTPCREL:
					case ELF_Def::R_X86_64_GOTPCRELX:
					case ELF_Def::R_X86_64_REX_GOTPCRELX:
						return G + GOT + A - P;

					case ELF_Def::R_X86_64_GOTOFF64:
						return S + A - GOT;

					case ELF_Def::R_X86_64_GOTPC32:
						return GOT + A - P;

					case ELF_Def::R_X86_64_SIZE32:
					case ELF_Def::R_X86_64_SIZE64:
						return Z + A;

					case ELF_Def::R_X86_64_IRELATIVE:
					{
						typedef uintptr_t (*indirect_t)();
						indirect_t func = reinterpret_cast<indirect_t>(B + A);
						return func();
					}

					default: // Not recognized!
						assert(false);
						return 0;
				}

				default: // Not recognized!
					assert(false);
					return 0;
			}
	}

	/*! \brief Get relocation value for internal symbol
	 * \param base Base address of the object to which this relocation belongs to
	 * \param global_offset_table address of the global offset table (in this object)
	 * \param plt_entry PLT entry of the symbol
	 */
	uintptr_t value(uintptr_t base, uintptr_t global_offset_table = 0, uintptr_t plt_entry = 0) const {
		return value(base, entry.symbol(), base, global_offset_table, plt_entry);
	}

	/*! \brief Get size of relocation value
	 * \return Size of relocation value
	 */
	size_t size() const {
		switch (entry.elf().header.machine()) {
			case ELF_Def::EM_386:
			case ELF_Def::EM_486:
				switch (type) {
					case ELF_Def::R_386_NONE:
					case ELF_Def::R_386_COPY:
						return 0;

					case ELF_Def::R_386_8:
					case ELF_Def::R_386_PC8:
						return 1;

					case ELF_Def::R_386_16:
					case ELF_Def::R_386_PC16:
						return 2;

					case ELF_Def::R_386_32:
					case ELF_Def::R_386_PC32:
					case ELF_Def::R_386_GOT32:
					case ELF_Def::R_386_PLT32:
					case ELF_Def::R_386_GLOB_DAT:
					case ELF_Def::R_386_JMP_SLOT:
					case ELF_Def::R_386_RELATIVE:
					case ELF_Def::R_386_GOTOFF:
					case ELF_Def::R_386_GOTPC:
					case ELF_Def::R_386_32PLT:
					case ELF_Def::R_386_SIZE32:
						return 4;

					default:  // Not recognized!
						assert(false);
						return 0;
				}

			case ELF_Def::EM_X86_64:
				switch (type) {
					case ELF_Def::R_X86_64_NONE:
					case ELF_Def::R_X86_64_COPY:
						return 0;

					case ELF_Def::R_X86_64_8:
					case ELF_Def::R_X86_64_PC8:
						return 1;

					case ELF_Def::R_X86_64_16:
					case ELF_Def::R_X86_64_PC16:
						return 2;

					case ELF_Def::R_X86_64_PC32:
					case ELF_Def::R_X86_64_GOT32:
					case ELF_Def::R_X86_64_PLT32:
					case ELF_Def::R_X86_64_GOTPCREL:
					case ELF_Def::R_X86_64_32:
					case ELF_Def::R_X86_64_32S:
					case ELF_Def::R_X86_64_TLSGD:
					case ELF_Def::R_X86_64_TLSLD:
					case ELF_Def::R_X86_64_DTPOFF32:
					case ELF_Def::R_X86_64_GOTTPOFF:
					case ELF_Def::R_X86_64_TPOFF32:
					case ELF_Def::R_X86_64_GOTPC32:
					case ELF_Def::R_X86_64_SIZE32:
					case ELF_Def::R_X86_64_GOTPC32_TLSDESC:
						return 4;

					case ELF_Def::R_X86_64_64:
					case ELF_Def::R_X86_64_RELATIVE64:
					case ELF_Def::R_X86_64_DTPMOD64:
					case ELF_Def::R_X86_64_DTPOFF64:
					case ELF_Def::R_X86_64_TPOFF64:
					case ELF_Def::R_X86_64_PC64:
					case ELF_Def::R_X86_64_GOTOFF64:
					case ELF_Def::R_X86_64_SIZE64:
					case ELF_Def::R_X86_64_GOT64:
					case ELF_Def::R_X86_64_GOTPCREL64:
					case ELF_Def::R_X86_64_GOTPC64:
					case ELF_Def::R_X86_64_GOTPLT64:
					case ELF_Def::R_X86_64_PLTOFF64:
						return 8;

					case ELF_Def::R_X86_64_GLOB_DAT:  // S
					case ELF_Def::R_X86_64_JUMP_SLOT:  // S
					case ELF_Def::R_X86_64_RELATIVE:
#ifdef __LP64__
						return 8;
#else // ILP32
						return 4;
#endif

					case ELF_Def::R_X86_64_TLSDESC:
						return 16;

					default:  // Not recognized!
						assert(false);
						return 0;
				}

			default:  // unsupported architecture
				assert(false);
				return 0;
		}
	}

	/*! \brief Fix relocation */
	uintptr_t apply_value(uintptr_t base, uintptr_t value) const {
		const uintptr_t mem = base + offset;

		switch (size()) {
			case 0: return 0;
			case 1: return write<uint8_t>(mem, value);
			case 2: return write<uint16_t>(mem, value);
			case 4: return write<uint32_t>(mem, value);
			case 8: return write<uint64_t>(mem, value);
			default:
				assert(false);
				return 0;
		}
	}

	/*! \brief Get relocation value for external symbol
	 * \param base Base address in target memory of the object to which this relocation belongs to
	 * \param symbol (External) Symbol in another object
	 * \param symbol_base base address of the object providing the external symbol
	 * \param global_offset_table address of the global offset table (in this object)
	 * \param plt_entry PLT entry of the symbol
	 */
	uintptr_t apply(uintptr_t base, const ELF_Def::Symbol & symbol, uintptr_t symbol_base, uintptr_t global_offset_table = 0, uintptr_t plt_entry = 0) const {
		assert(symbol.section_index() != ELF_Def::SHN_UNDEF);
		return relocate_value(base, value(base, symbol, symbol_base, global_offset_table, plt_entry));
	}

	/*! \brief Get relocation value (for internal symbol)
	 * \param base Base address in target memory of the object to which this relocation belongs to
	 * \param global_offset_table address of the global offset table
	 * \param plt_entry PLT entry of the symbol
	 */
	uintptr_t apply(uintptr_t base, uintptr_t global_offset_table = 0, uintptr_t plt_entry = 0) const {
		assert(symbol.section_index() != ELF_Def::SHN_UNDEF);
		return relocate_value(base, value(base, global_offset_table, plt_entry));
	}

 private:
	/*! \brief Write at a specific memory address
	* \tparam T size of value
	* \param mem memory address
	* \param value value to write at memory address with given size
	* \return written value
	*/
	template<typename T>
	static uintptr_t write(uintptr_t mem, uintptr_t value) {
		T v = static_cast<T>(value);
		T * m = reinterpret_cast<T *>(mem);
		*m = v;
		assert(static_cast<intptr_t>(v) == static_cast<intptr_t>(value));
		return static_cast<uintptr_t>(v);
	}
};
