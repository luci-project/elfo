#pragma once

#include "elf.hpp"

/*! \brief Calculate relocation */
template<typename RELOC>
struct Relocator : private ELF_Def::Constants {
	using RELF = decltype(RELOC::_elf);

	/*! \brief Offset in object to be relocated */
	const RELOC & entry;

	/*! \brief address of the global offset table */
	const uintptr_t global_offset_table;

	/*! \brief Constructor
	 * \param entry Relocation
	 * \param global_offset_table address of the global offset table (in this object)
	 */
	Relocator(const RELOC & entry, uintptr_t global_offset_table = 0)
	  : entry(entry), global_offset_table(global_offset_table) {
		assert(entry.valid());
	}

	/*! \brief Check if copy relocation
	 * \param type Relocation type
	 * \param machine Elf target machine
	 * \return `true` if copy relocation
	 */
	static bool is_copy(uintptr_t type, ehdr_machine machine) {
		switch (machine) {
			case EM_386:
			case EM_486:
				return type == R_386_COPY;

			case EM_X86_64:
				return type == R_X86_64_COPY;

			default:  // unsupported architecture
				assert(false);
				return 0;
		}
	}

	/*! \brief Check if this is a copy relocation
	 * \return `true` if copy relocation
	 */
	bool is_copy() const {
		return is_copy(entry.type(), entry.elf().header.machine());
	}

	/*! \brief Check if indirect relocation
	 * \param type Relocation type
	 * \param machine Elf target machine
	 * \return `true` if indirect relocation
	 */
	static bool is_indirect(uintptr_t type, ehdr_machine machine) {
		switch (machine) {
			case EM_386:
			case EM_486:
				return type == R_386_IRELATIVE;

			case EM_X86_64:
				return type == R_X86_64_IRELATIVE;

			default:  // unsupported architecture
				assert(false);
				return 0;
		}
	}

	/*! \brief Check if this is a indirect relocation
	 * \return `true` if indirect relocation
	 */
	bool is_indirect() const {
		return is_indirect(entry.type(), entry.elf().header.machine());
	}


	/*! \brief Get relocation fix value
	 * \note neither copy is performed nor an indirection function is called
	 * \param base Base address in target memory of the object to which this relocation belongs to
	 * \param symbol (External) Symbol (from another object)
	 * \param symbol_base base address of the object providing the external symbol
	 * \param plt_entry PLT entry of the symbol
	 * \param tls_module_id TLS module ID of (external) symbol
	 * \param tls_offset TLS offset (from thread pointer / %fs) of (external) symbol
	 */
	template<typename Symbol>
	uintptr_t value(uintptr_t base, const Symbol & symbol, uintptr_t symbol_base, uintptr_t plt_entry = 0, uintptr_t tls_module_id = 0, intptr_t tls_offset = 0) const {
		const intptr_t A = entry.addend();
		const uintptr_t B = base;
		const uintptr_t G = symbol_base + symbol.value();
		const uintptr_t GOT = global_offset_table;
		const uintptr_t L = plt_entry;
		const uintptr_t P = address(base);
		const uintptr_t S = symbol_base + symbol.value();
		const uintptr_t Z = symbol.size();

		switch (entry.elf().header.machine()) {
			case EM_386:
			case EM_486:
			switch (entry.type()) {
				case R_386_NONE:
					return 0;

				case R_386_COPY:
					return S;

				case R_386_8:
				case R_386_16:
				case R_386_32:
					return S + A;

				case R_386_PC8:
				case R_386_PC16:
				case R_386_PC32:
					return S + A - P;

				case R_386_GOT32:
					return G + A;

				case R_386_PLT32:
					assert(L != 0);
					return L + A - P;

				case R_386_GLOB_DAT:
				case R_386_JMP_SLOT:
					assert(A == 0);
					return S;

				case R_386_RELATIVE:
				case R_386_IRELATIVE:
					return B + A;

				case R_386_GOTOFF:
					return S + A - GOT;

				case R_386_GOTPC:
					return GOT + A - P;

				case R_386_32PLT:
					assert(L != 0);
					return L + A;

				case R_386_SIZE32:
					return Z + A;

				default: // Not recognized!
					assert(false);
					return 0;
			}

			case EM_X86_64:
				switch (entry.type()) {
					case R_X86_64_NONE:
						return 0;

					case R_X86_64_COPY:
						return S;

					case R_X86_64_GLOB_DAT:
					case R_X86_64_JUMP_SLOT:
						assert(A == 0);
						return S;

					case R_X86_64_8:
					case R_X86_64_16:
					case R_X86_64_32:
					case R_X86_64_32S:
					case R_X86_64_64:
						return S + A;

					case R_X86_64_PC8:
					case R_X86_64_PC16:
					case R_X86_64_PC32:
					case R_X86_64_PC64:
						return S + A - P;

					case R_X86_64_GOT32:
						return G + A;

					case R_X86_64_PLT32:
						assert(L != 0);
						return L + A - P;

					case R_X86_64_RELATIVE:
					case R_X86_64_RELATIVE64:
					case R_X86_64_IRELATIVE:
						return B + A;

					case R_X86_64_GOTPCREL:
					case R_X86_64_GOTPCRELX:
					case R_X86_64_REX_GOTPCRELX:
						return G + GOT + A - P;

					case R_X86_64_GOTOFF64:
						return S + A - GOT;

					case R_X86_64_GOTPC32:
						return GOT + A - P;

					case R_X86_64_SIZE32:
					case R_X86_64_SIZE64:
						return Z + A;

					case R_X86_64_TPOFF64:
						assert(tls_module_id != 0 && tls_offset != 0);
						return symbol.value() + A - tls_offset;

					case R_X86_64_DTPMOD64:
						return tls_module_id;

					case R_X86_64_DTPOFF64:
						return symbol.value() + A;


					default: // Not recognized!
						assert(false);
						return 0;
				}

				default: // Not recognized!
					assert(false);
					return 0;
			}
	}

	inline uintptr_t value(uintptr_t base, uintptr_t plt_entry = 0, uintptr_t tls_module_id = 0, intptr_t tls_offset = 0) const {
		return value(base, entry.symbol(), base, plt_entry, tls_module_id, tls_offset);
	}

	/*! \brief Get relocation fix value (for external symbol)
	 * \note Perform memcopy or call indirect function if required
	 * \param base Base address in target memory of the object to which this relocation belongs to
	 * \param symbol (External) Symbol (from another object)
	 * \param symbol_base base address of the object providing the external symbol
	 * \param plt_entry PLT entry of the symbol
	 * \param tls_module_id TLS module ID of (external) symbol
	 * \param tls_offset TLS offset (from thread pointer / %fs) of (external) symbol
	 */
	template<typename Symbol>
	inline uintptr_t value_external(uintptr_t base, const Symbol & symbol, uintptr_t symbol_base, uintptr_t plt_entry = 0, uintptr_t tls_module_id = 0, intptr_t tls_offset = 0) const {
		auto v = value(base, symbol, symbol_base, plt_entry, tls_module_id, tls_offset);
		if (is_copy()) {
			memcpy(reinterpret_cast<void*>(address(base)), reinterpret_cast<void*>(v), symbol.size());
			return 0;
		} else if (symbol.type() == STT_GNU_IFUNC || is_indirect()) {
			typedef uintptr_t (*indirect_t)();
			indirect_t func = reinterpret_cast<indirect_t>(v);
			auto r = func();
			return r;
		} else {
			return v;
		}
	}

	/*! \brief Get relocation fix value for internal symbol
	 * \param base Base address of the object to which this relocation belongs to
	 * \param plt_entry PLT entry of the symbol
	 * \param tls_module_id TLS module ID of (external) symbol
	 * \param tls_offset TLS offset (from thread pointer / %fs) of (external) symbol
	 */
	inline uintptr_t value_internal(uintptr_t base, uintptr_t plt_entry = 0, uintptr_t tls_module_id = 0, intptr_t tls_offset = 0) const {
		return value_external(base, entry.symbol(), base, plt_entry, tls_module_id, tls_offset);
	}

	/*! \brief Get size of relocation value
	 * \return Size of relocation value
	 */
	inline size_t size() const {
		return size(entry.type(), entry.elf().header.machine());
	}

	/*! \brief Get size of relocation value
	 * \param type Relocation type
	 * \param machine Elf target machine
	 * \return Size of relocation value
	 */
	static size_t size(uintptr_t type, ehdr_machine machine) {
		switch (machine) {
			case EM_386:
			case EM_486:
				switch (type) {
					case R_386_NONE:
					case R_386_COPY:
						return 0;

					case R_386_8:
					case R_386_PC8:
						return 1;

					case R_386_16:
					case R_386_PC16:
						return 2;

					case R_386_32:
					case R_386_PC32:
					case R_386_GOT32:
					case R_386_PLT32:
					case R_386_GLOB_DAT:
					case R_386_JMP_SLOT:
					case R_386_RELATIVE:
					case R_386_GOTOFF:
					case R_386_GOTPC:
					case R_386_32PLT:
					case R_386_SIZE32:
						return 4;

					default:  // Not recognized!
						assert(false);
						return 0;
				}

			case EM_X86_64:
				switch (type) {
					case R_X86_64_NONE:
					case R_X86_64_COPY:
						return 0;

					case R_X86_64_8:
					case R_X86_64_PC8:
						return 1;

					case R_X86_64_16:
					case R_X86_64_PC16:
						return 2;

					case R_X86_64_PC32:
					case R_X86_64_GOT32:
					case R_X86_64_PLT32:
					case R_X86_64_GOTPCREL:
					case R_X86_64_32:
					case R_X86_64_32S:
					case R_X86_64_TLSGD:
					case R_X86_64_TLSLD:
					case R_X86_64_DTPOFF32:
					case R_X86_64_GOTTPOFF:
					case R_X86_64_TPOFF32:
					case R_X86_64_GOTPC32:
					case R_X86_64_SIZE32:
					case R_X86_64_GOTPC32_TLSDESC:
						return 4;

					case R_X86_64_64:
					case R_X86_64_RELATIVE64:
					case R_X86_64_DTPMOD64:
					case R_X86_64_DTPOFF64:
					case R_X86_64_TPOFF64:
					case R_X86_64_PC64:
					case R_X86_64_GOTOFF64:
					case R_X86_64_SIZE64:
					case R_X86_64_GOT64:
					case R_X86_64_GOTPCREL64:
					case R_X86_64_GOTPC64:
					case R_X86_64_GOTPLT64:
					case R_X86_64_PLTOFF64:
						return 8;

					case R_X86_64_GLOB_DAT:
					case R_X86_64_JUMP_SLOT:
					case R_X86_64_RELATIVE:
					case R_X86_64_IRELATIVE:
#ifdef __LP64__
						return 8;
#else // ILP32
						return 4;
#endif

					case R_X86_64_TLSDESC:
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

	/*! \brief Target address for relocation
	 * \param base base address
	 * \return target address
	 */
	inline uintptr_t address(uintptr_t base) const {
		return base + entry.offset();
	}

	/*! \brief Read the current value in target address
	 * \param base base address
	 * \return current value
	 */
	uintptr_t read_value(uintptr_t base) const {
		const uintptr_t mem = address(base);

		switch (size()) {
			case 0: return 0;
			case 1: return read<uint8_t>(mem);
			case 2: return read<uint16_t>(mem);
			case 4: return read<uint32_t>(mem);
			case 8: return read<uint64_t>(mem);
			default:
				assert(false);
				return 0;
		}
	}

	/*! \brief Write a new value to target address
	 * \param base base address
	 * \param value new value to write at target
	 * \return new value
	 */
	uintptr_t write_value(uintptr_t base, uintptr_t value) const {
		const uintptr_t mem = address(base);

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

	/*! \brief Increment target value
	 * \param base base address
	 * \param delta_value delta to add to value
	 * \return new value
	 */
	uintptr_t increment_value(uintptr_t base, uintptr_t delta_value) const {
		const uintptr_t mem = address(base);

		switch (size()) {
			case 0: return 0;
			case 1: return increment<uint8_t>(mem, delta_value);
			case 2: return increment<uint16_t>(mem, delta_value);
			case 4: return increment<uint32_t>(mem, delta_value);
			case 8: return increment<uint64_t>(mem, delta_value);
			default:
				assert(false);
				return 0;
		}
	}

	/*! \brief Calculate and apply relocation for external symbol
	 * \param base Base address in target memory of the object to which this relocation belongs to
	 * \param symbol (External) Symbol in another object
	 * \param symbol_base base address of the object providing the external symbol
	 * \param plt_entry PLT entry of the symbol
	 * \param tls_module_id TLS module ID of (external) symbol
	 * \param tls_offset TLS offset (from thread pointer / %fs) of (external) symbol
	 */
	template<typename Symbol>
	inline uintptr_t fix_external(uintptr_t base, const Symbol & symbol, uintptr_t symbol_base, uintptr_t plt_entry = 0, uintptr_t tls_module_id = 0, intptr_t tls_offset = 0) const {
		assert(symbol.section_index() != SHN_UNDEF);
		return write_value(base, value_external(base, symbol, symbol_base, plt_entry, tls_module_id, tls_offset));
	}

	/*! \brief Calculate and apply relocation (for internal symbol)
	 * \param base Base address in target memory of the object to which this relocation belongs to
	 * \param plt_entry PLT entry of the symbol
	 * \param tls_module_id TLS module ID of (external) symbol
	 * \param tls_offset TLS offset (from thread pointer / %fs) of (external) symbol
	 */
	inline uintptr_t fix_internal(uintptr_t base, uintptr_t plt_entry = 0, uintptr_t tls_module_id = 0, intptr_t tls_offset = 0) const {
		assert(entry.symbol().section_index() == SHN_UNDEF);
		return write_value(base, value_internal(base, plt_entry, tls_module_id, tls_offset));
	}

 private:
	/*! \brief Read from a specific memory address
	* \tparam T size of value
	* \param mem memory address
	* \return current value
	*/
	template<typename T>
	static inline uintptr_t read(uintptr_t mem) {
		T * m = reinterpret_cast<T *>(mem);
		return static_cast<uintptr_t>(*m);
	}

	/*! \brief Write at a specific memory address
	* \tparam T size of value
	* \param mem memory address
	* \param value value to write at memory address with given size
	* \return written value
	*/
	template<typename T>
	static inline uintptr_t write(uintptr_t mem, uintptr_t value) {
		T v = static_cast<T>(value);
		T * m = reinterpret_cast<T *>(mem);
		*m = v;
		assert(static_cast<intptr_t>(v) == static_cast<intptr_t>(value));
		return static_cast<uintptr_t>(v);
	}

	/*! \brief Increment the contents of a specific memory address
	* \tparam T size of value
	* \param mem memory address
	* \param delta_value value to add to the contents
	* \return written value
	*/
	template<typename T>
	static inline uintptr_t increment(uintptr_t mem, uintptr_t delta_value) {
		T v = static_cast<T>(delta_value);
		T * m = reinterpret_cast<T *>(mem);
		*m += v;
		return static_cast<uintptr_t>(*m);
	}
};
