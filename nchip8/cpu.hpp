//
// Created by ocanty on 25/09/18.
//

#ifndef CHIP8_NCURSES_CPU_HPP
#define CHIP8_NCURSES_CPU_HPP

#include <array>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>

namespace nchip8
{

//! The CHIP-8 interpreter core
class cpu
{
public:
    cpu();

    ~cpu();

    //! @brief  Clears RAM, registers, the stack, screen etc...
    void reset();

    //! @brief              Loads a ROM into the CPU
    //! @param  rom         A vector of 8-bit ints of the raw rom data
    //! @param  address     The memory location to load the rom into
    //! @returns            true if loading was successful, false otherwise
    bool load_rom(const std::vector<std::uint8_t> &rom, const std::uint16_t address);

    //! @brief Executes the current instruction at PC, (PC may jump or increment afterwards)
    void execute_op_at_pc();

    //! @brief          Returns a disassembly of the instruction at the supplied address
    //! @param address  The address of the instruction, must be correctly aligned
    //! @returns        Optional of string of disassembled instruction
    std::optional<std::string> dasm_op(const std::uint16_t &address) const;

private:
    //! RAM
    std::array<std::uint8_t, 0x1000> m_ram;

    //! General Purpose Registers
    std::array<std::uint8_t, 16> m_gpr;

    //! I register, for storing addresses for some special instructions
    std::uint16_t m_i;

    //! Program Counter, the address of the current executing instruction
    std::uint16_t m_pc;

    //! Stack Pointer, the size of the stack
    std::uint8_t m_sp;

    //! The Stack
    std::array<std::uint16_t, 16> m_stack;

    //! @brief          Reads a 16-bit value at the specified address
    //! @param address  The address
    std::uint16_t read_u16(const std::uint16_t &address) const;

    //! @brief          Sets a 16-bit value at the specified address
    //! @param address  The address
    //! @param val      The new value
    void set_u16(const std::uint16_t &address, const std::uint16_t &val);

    //! @brief      The operand data that an instruction may have, passed to execute/dasm functions
    //! @details    CHIP-8 instruction breakdown:
    //!             Any CHIP-8 instruction can take the following forms:
    //!             0xANNN, 0xAKKA, 0xAXYA, 0xAAAN, 0xAXYN, 0xAAAA
    //!             where A is strictly defined (i.e specifies the instruction)
    //!             NNN, X, Y, KK, and N are considered operand values
    struct operand_data
    {
        std::uint16_t m_nnn; //! 0xANNN where A is part of opcode and NNN is data
        std::uint8_t m_x;   //! 0xAXAA where A is part of opcode and X is data
        std::uint8_t m_y;   //! 0xAAYA where A is part of opcode and Y is data
        std::uint8_t m_kk;  //! 0xAAKK where A is part of opcode and KK is data
        std::uint8_t m_n;   //! 0xAAAN where A is part of opcode and N is data
    };

    //! @brief  A function type that when executed,
    //!         should process the instruction operation and update the relevant parts of the CPU
    using func_execute_op = std::function<void(cpu &, const operand_data &)>;

    //! @brief  A function that when called,
    //!         returns the disassembly string of the instruction
    using func_dasm_op = std::function<void(const operand_data &, std::stringstream &)>;

    //! @brief Container type to hold both functions that could process an instruction
    //!        both an execution and a disassembly routine
    struct op_handler
    {
        //! An array specifying the instruction encoding, operand data is indexed by std::nullopt
        //! e.g. 0x1NNN - { 0x1, std::nullopt, std::nullopt, std::nullopt }
        std::array<std::optional<std::uint8_t>, 4> m_encoding;

        //! @see func_execute_op
        func_execute_op m_execute_op;

        //! @see func_dasm_op
        func_dasm_op m_dasm_op;
    };

    friend class op_handler; //! We allow operations to access data in CPU (i.e its private members)

    //! @brief      The operation handler tree
    //!             4 nested maps, indexed by each nibble of the instruction
    //!             e.g. 0xABCD, m_op_tree[A][B][C][D]
    //!
    //! @details    4bit nibbles in this case are using an 8bit type
    //!             Operand data is indexed as optional (std::nullopt)
    std::unordered_map<std::optional<std::uint8_t>,
            std::unordered_map<std::optional<std::uint8_t>,
                    std::unordered_map<std::optional<std::uint8_t>,
                            std::unordered_map<std::optional<std::uint8_t>,
                                    op_handler>>>> m_op_tree;

    //! @brief          Returns the operation handler for an instruction
    //! @param address  The encoded instruction (i.e 0X1200 - JP 200)
    //! @returns        Optional of operation handler if successful, std::nullopt if false
    std::optional<op_handler> get_op_handler_for_instruction(const std::uint16_t &instruction);

    //! @brief          Add an operation handler for an instruction into the handler tree
    //! @param handler  Handler structure, containing an execute and disassembly function
    void add_op_handler(const op_handler &handler);

    /* Begin operation handlers
       Why are these not stored inside an array? We want to alias them.
       Rationale: Easier to write unit tests
                  if we can simply call instructions by name
                  as the instruction name will match the test name */
    static op_handler CLS;          // 00E0 - CLS
    static op_handler RET;          // 00EE - RET
    static op_handler SYS;          // 0nnn - SYS addr
    static op_handler JP;           // 1nnn - JP addr
    static op_handler CALL;         // 2nnn - CALL addr
    static op_handler SE_VX_KK;     // 3xkk - SE Vx, byte
    static op_handler SNE_VX_KK;    // 4xkk - SNE Vx, byte
    static op_handler SE_VX_VY;     // 5xy0 - SE Vx, Vy
    static op_handler LD_VX_KK;     // 6xkk - LD Vx, byte
    static op_handler ADD_VX_KK;    // 7xkk - ADD Vx, byte
    static op_handler LD_VX_VY;     // 8xy0 - LD Vx, Vy
    static op_handler OR_VX_Y;      // 8xy1 - OR Vx, Vy
    static op_handler AND_VX_VY;    // 8xy2 - AND Vx, Vy
    static op_handler XOR_VX_VY;    // 8xy3 - XOR Vx, Vy
    static op_handler ADD_VX_VY;    // 8xy4 - ADD Vx, Vy
    static op_handler SUB_VX_VY;    // 8xy5 - SUB Vx, Vy
    static op_handler SHR_VX_VY;    // 8xy6 - SHR Vx {, Vy}
    static op_handler SUBN_VX_VY;   // 8xy7 - SUBN Vx, Vy
    static op_handler SHL_VX_VY;    // 8xyE - SHL Vx {, Vy}
    static op_handler SNE_VX_VY;    // 9xy0 - SNE Vx, Vy
    static op_handler LD_I_NNN;     // Annn - LD I, addr
    static op_handler JP_V0_NNN;    // Bnnn - JP V0, addr
    static op_handler RND_VX_KK;    // Cxkk - RND Vx, byte
    static op_handler DRW_VX_VY_N;  // Dxyn - DRW Vx, Vy, nibble
    static op_handler SKP_VX;       // Ex9E - SKP Vx
    static op_handler SKNP_VX;      // ExA1 - SKNP Vx
    static op_handler LD_VX_DT;     // Fx07 - LD Vx, DT
    static op_handler LD_VX_K;      // Fx0A - LD Vx, K
    static op_handler LD_DT_VX;     // Fx15 - LD DT, Vx
    static op_handler LD_ST_VX;     // Fx18 - LD ST, Vx
    static op_handler ADD_I_VX;     // Fx1E - ADD I, Vx
    static op_handler LD_F_VX;      // Fx29 - LD F, Vx
    static op_handler LD_B_VX;      // Fx33 - LD B, Vx
    static op_handler LD_imm_I_VX;  // Fx55 - LD [I], Vx
    static op_handler LD_VX_imm_I;  // Fx65 - LD Vx,
    /* End operation handlers */

    //! @brief Add all the CHIP-8 operation handlers to the operation tree
    void setup_op_handlers();

};

}

#endif //CHIP8_NCURSES_CPU_HPP
