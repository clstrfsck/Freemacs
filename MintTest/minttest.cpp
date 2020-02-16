/*
 * Copyright 2020 Martin Sandiford
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to: Free Software Foundation
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <iostream>

#include "mintprim.h"

#include "bufprim.h"
#include "frmprim.h"
#include "libprim.h"
#include "mthprim.h"
#include "strprim.h"
#include "sysprim.h"

#include "gtest.h"

// External in mint.cpp
bool keyWaiting() {
    return false;
} // keyWaiting

namespace {

    const std::string OK("OK");
    const std::string BAD("BAD");

    class owPrim : public MintPrim {
    public:
        owPrim(std::ostream &out) : _out(out) {
            // Nothing
        }

        void operator()(Mint& interp, bool is_active, const MintArgList& args) {
            MintArgList::const_iterator i = args.begin();
            if (i != args.end()) {
                // We are skipping arg 0
                ++i;
                while (i != args.end()) {
                    _out << i->getValue();
                    ++i;
                } // while
            } // if
            interp.returnNull(is_active);
        } // operator()

    private:
        std::ostream &_out;
    }; // owPrim

    class MintTest : public Mint {
    public:
        MintTest(const char *output) : Mint(output) {
            addPrim("ow", new owPrim(_out));

            // Basic test cases written for these primitives
            registerMthPrims(*this);
            registerStrPrims(*this);
            registerFrmPrims(*this);

            // FIXME: Test cases yet to be written for these
            registerSysPrims(*this, 0, 0, 0);
            registerLibPrims(*this);
            registerBufPrims(*this);
        }

        std::string result() {
            scan();
            return _out.str();
        }

    private:
        std::stringstream _out;
    }; // MintTest
}

TEST(Testing, owPrim) {
    EXPECT_EQ(OK, MintTest("#(ow,OK)").result());
}

//
// Primitives from mthprim.cpp
//

TEST(MthPrim, bcPrim) {
    // Character to dec, oct, hex, bin
    EXPECT_EQ("64", MintTest("#(ow,#(bc,@,a,d))").result());
    EXPECT_EQ("64", MintTest("#(ow,#(bc,@,c,d))").result());
    EXPECT_EQ("100", MintTest("#(ow,#(bc,@,c,o))").result());
    EXPECT_EQ("40", MintTest("#(ow,#(bc,@,c,h))").result());
    EXPECT_EQ("1000000", MintTest("#(ow,#(bc,@,c,b))").result());

    // Decimal to character, oct, hex, bin
    EXPECT_EQ("A", MintTest("#(ow,#(bc,65,d,a))").result());
    EXPECT_EQ("A", MintTest("#(ow,#(bc,65,d,c))").result());
    EXPECT_EQ("101", MintTest("#(ow,#(bc,65,d,o))").result());
    EXPECT_EQ("41", MintTest("#(ow,#(bc,65,d,h))").result());
    EXPECT_EQ("1000001", MintTest("#(ow,#(bc,65,d,b))").result());
}

TEST(MthPrim, addPrim) {
    EXPECT_EQ("Prefix 15", MintTest("#(ow,##(++,(Prefix 12),3))").result());
}

TEST(MthPrim, subPrim) {
    EXPECT_EQ("Prefix 9", MintTest("#(ow,##(--,(Prefix 12),3))").result());
}

TEST(MthPrim, mulPrim) {
    EXPECT_EQ("Prefix 36", MintTest("#(ow,##(**,(Prefix 12),3))").result());
}

TEST(MthPrim, divPrim) {
    EXPECT_EQ("Prefix 4", MintTest("#(ow,##(//,(Prefix 12),3))").result());
}

TEST(MthPrim, modPrim) {
    EXPECT_EQ("Prefix 1", MintTest("#(ow,##(%%,(Prefix 13),3))").result());
}

TEST(MthPrim, iorPrim) {
    EXPECT_EQ("Prefix 15", MintTest("#(ow,##(||,(Prefix 13),3))").result());
}

TEST(MthPrim, andPrim) {
    EXPECT_EQ("Prefix 1", MintTest("#(ow,##(&&,(Prefix 13),3))").result());
}

TEST(MthPrim, xorPrim) {
    EXPECT_EQ("Prefix 14", MintTest("#(ow,##(^^,(Prefix 13),3))").result());
}

TEST(MthPrim, gtPrim) {
    EXPECT_EQ(OK, MintTest("#(ow,#(g?,9,10,BAD,OK))").result());
    EXPECT_EQ(OK, MintTest("#(ow,#(g?,10,9,OK,BAD))").result());
}


//
// Primitives from strprim.cpp
//

TEST(StrPrim, eqPrim) { // ==
    EXPECT_EQ(OK, MintTest("#(ow,#(==,A,A,OK,BAD))").result());
    EXPECT_EQ(OK, MintTest("#(ow,#(==,A,B,BAD,OK))").result());
}

TEST(StrPrim, nePrim) { // !=
    EXPECT_EQ(OK, MintTest("#(ow,#(!=,A,A,BAD,OK))").result());
    EXPECT_EQ(OK, MintTest("#(ow,#(!=,A,B,OK,BAD))").result());
}

TEST(StrPrim, ncPrim) {
    EXPECT_EQ("5", MintTest("#(ow,#(nc,hello))").result());
    EXPECT_EQ("11", MintTest("#(ow,#(nc,hello hello))").result());
}

TEST(StrPrim, aoPrim) { // a?
    EXPECT_EQ(OK, MintTest("#(ow,#(a?,A,A,OK,BAD))").result());
    EXPECT_EQ(OK, MintTest("#(ow,#(a?,A,B,OK,BAD))").result());
    EXPECT_EQ(OK, MintTest("#(ow,#(a?,AA,A,BAD,OK))").result());
}

TEST(StrPrim, saPrim) {
    EXPECT_EQ("b,c,m,n,v,x,z", MintTest("#(ow,##(sa,z,x,c,v,b,n,m))").result());
}

TEST(StrPrim, siPrim) {
    EXPECT_EQ("A0123456789Z", MintTest(
                  "#(ds,xlat,(z0123456789))"
                  "#(ow,##(si,xlat,(A\001\002\003\004\005\006\007\010\011\012Z)))").result());
}

TEST(StrPrim, nlPrim) {
    EXPECT_EQ("\n", MintTest("#(ow,##(nl))").result());
}


//
// Primitives from frmprim.cpp
//

TEST(FrmPrim, dsPrim) {
    EXPECT_EQ("Test string", MintTest("#(ds,zz,Test string)#(ow,#(zz))").result());
    EXPECT_EQ("Test string", MintTest("#(ds,zz,Test string)#(ow,##(zz))").result());
}

TEST(FrmPrim, gsPrim) {
    EXPECT_EQ("Test string", MintTest("#(ds,zz,Test string)#(ow,#(gs,zz))").result());
    EXPECT_EQ("Test string", MintTest("#(ds,zz,Test string)#(ow,##(gs,zz))").result());
}

TEST(FrmPrim, goPrim) {
    EXPECT_EQ("", MintTest("#(ds,zz,AB)#(ow,##(go,zzz,OK))").result());
    EXPECT_EQ("A", MintTest("#(ds,zz,AB)#(ow,#(go,zz,OK))").result());
    EXPECT_EQ("A", MintTest("#(ds,zz,AB)#(ow,##(go,zz,OK))").result());
    EXPECT_EQ("ABOK", MintTest("#(ds,zz,AB)#(ow,##(go,zz,OK)##(go,zz,OK)##(go,zz,OK))").result());
    EXPECT_EQ("AOKB", MintTest("#(ds,zz,AB)#(ow,##(go,zz,OK)OK##(gs,zz))").result());
}

TEST(FrmPrim, gnPrim) {
    // Behaviour as implemented (here, and in original Freemacs) does
    // not match the documentation in primitives.txt.  Implemented
    // behaviour is described below.

    // #(gn,X,Y,Z)
    // ---------
    // Get n.  Gets "Y" characters from form "X".  If there are less
    // than "Y" characters remaining between the form pointer and the
    // end of the form, fewer than "Y" characters will be returned. If
    // the form cannot be found, the null string is returned.  If the
    // form is found, and the form pointer is currently at the end of
    // the form, string "Z" is returned in active mode.  This is
    // approximately equivalent to the TRAC #(cn,...) primitive, only
    // argument markers appear to be returned in MINT.
    // Returns: Up to "Y" characters from the form at the form pointer.

    // Code from Freemacs for gn primitive for comparison is as follows:
    //
    // gn_prim:
    //         call    find_arg1
    //         jc      gn_prim_1       ;arg1 form not found -> null
    //         assume  ds:formSeg
    //         jcxz    gn_prim_2       ;form pointer empty -> #(arg3)
    //         push    ds              ;save pointer, count to form.
    //         push    si
    //         push    cx
    //         push    bx
    //         dsdata
    //         mov     cx,2            ;get number of chars to call.
    //         call    get_decimal_arg
    //         mov     dx,ax           ;save in dx.
    //         pop     bx
    //         pop     cx
    //         pop     si
    //         pop     ds
    //         assume  ds:formSeg
    //         di_points_fbgn
    //         cmp     dx,cx           ;are we trying to get more than exists?
    //         jbe     gn_prim_3       ;no - move the requested amount.
    //         mov     dx,cx           ;yes - truncate the count.
    // gn_prim_3:
    //         xchg    dx,cx           ;swap the count remaining and the get count.
    //         sub     dx,cx           ;dec the count remaining by the get count.
    //         chk_room_cnt es         ;check for collision
    //         movmem                  ;move all the chars.
    //         mov     cx,dx           ;return the count remaining in cx.
    //         jmp     return_form
    // gn_prim_2:
    //         dsdata
    //         mov     cx,3
    //         jmp     return_arg_active
    // gn_prim_1:
    //         jmp     return_null
    //         assume  ds:data, es:data

    EXPECT_EQ("", MintTest("#(ds,zz,AB)#(ow,#(gn,zzz,1,BAD))").result());
    EXPECT_EQ("A", MintTest("#(ds,zz,AB)#(ow,#(gn,zz,1,BAD))").result());
    EXPECT_EQ("A", MintTest("#(ds,zz,AB)#(ow,##(gn,zz,1,BAD))").result());
    EXPECT_EQ("ABOK", MintTest("#(ds,zz,AB)#(ow,##(gn,zz,2,BAD)##(gn,zz,2,OK))").result());
    EXPECT_EQ("AOKB", MintTest("#(ds,zz,AB)#(ow,##(gn,zz,1,BAD)OK##(gs,zz))").result());
}

TEST(FrmPrim, rsPrim) {
    EXPECT_EQ("AAB", MintTest("#(ow,#(ds,zz,AB)#(go,zz,BAD)#(rs,zz)#(gs,zz,BAD))").result());
}

TEST(FrmPrim, fmPrim) {
    EXPECT_EQ("AC", MintTest("#(ow,#(ds,zz,ABC)#(fm,zz,B,BAD)#(gs,zz,BAD))").result());
    EXPECT_EQ("", MintTest("#(ow,#(ds,zz,ABC)#(fm,zzz,B,BAD))").result());
    EXPECT_EQ("OK", MintTest("#(ow,#(ds,zz,ABC)#(fm,zz,,OK))").result());
    EXPECT_EQ("OK", MintTest("#(ow,#(ds,zz,ABC)#(fm,zz,D,OK))").result());
}

TEST(FrmPrim, nxPrim) {
    EXPECT_EQ("OK", MintTest("#(ow,#(ds,zz,ABC)#(n?,zz,OK,BAD))").result());
    EXPECT_EQ("OK", MintTest("#(ow,#(ds,zz,ABC)#(n?,zzz,BAD,OK))").result());
}

TEST(FrmPrim, lsPrim) {
    EXPECT_EQ("z,zz,zzz", MintTest("#(ow,#(ds,z,ABC)#(ds,zz,ABC)#(ds,zzz,ABC)##(sa,#(ls,(,),z)))").result());
}

TEST(FrmPrim, esPrim) {
    EXPECT_EQ("OKOK", MintTest("#(ow,#(ds,zz,ABC)#(ds,zzz,ABC)#(es,zz)#(n?,zz,BAD,OK)#(n?,zzz,OK,BAD))").result());
    EXPECT_EQ("OKOK", MintTest("#(ow,#(ds,zz,ABC)#(ds,zzz,ABC)#(es,zz,zzz)#(n?,zz,BAD,OK)#(n?,zzz,BAD,OK))").result());
}

TEST(FrmPrim, mpPrim) {
    EXPECT_EQ("Test test,A,B,C", MintTest(
                  "#(ow,"
                  "#(ds,test,(Test SELF,ARG1,ARG2,ARG3))"
                  "#(mp,test,SELF,ARG1,ARG2,ARG3)"
                  "##(test,A,B,C)"
                  ")").result());
}

TEST(FrmPrim, hkPrim) {
    EXPECT_EQ(OK, MintTest(
                  "#(ow,"
                  "#(ds,z1,OK)"
                  "##(hk,aa,bb,cc,dd,z1)"
                  ")").result());
}




int zmain(int argc, char **argv, char **envp) {
    try {
        Mint interp(
            "#(ds,zz,(Fish fingers))"
            "#(ds,z1,(This is z1))"
            "#(ds,z2,(This is z2))"
            "#(ds,z3,(This is z3))"
            "#(ds,z4,(This is z4))"
            "#(ds,z5,(This is z5))"
            "#(ds,z6,(This is z6))"
            "#(ds,z7,(This is z7))"
            "#(ds,z8,(This is z8))"
            "#(ds,z9,(This is z9))"
            "#(ow,(##(zz) = )##(zz)(\n))"
            "#(ow,(##(gs,zz) = )##(gs,zz)(\n))"
            "#(ow,(##(go,zz) = )##(go,zz)(\n))"
            "#(ow,(##(go,zz) = )##(go,zz)(\n))"
            "#(ow,(##(gn,zz,5) = )##(gn,zz,5)(\n))"
            "#(ow,(##(gn,zz,5) = )##(gn,zz,5)(\n))"
            "#(ow,(##(go,zz,Failed) = )##(go,zz,Failed)(\n))"
            "#(ow,(##(go,kk,Failed) = )##(go,kk,Failed)(\n))"
            "#(rs,zz)#(ow,(##(gn,zz,4) = )##(gn,zz,4)(\n))"
            "#(ow,(##(fm,zz,h,Failed) = )##(fm,zz,h,Failed)(\n))"
            "#(rs,zz)#(ow,(##(fm,zz,h) = )##(fm,zz,h)(\n))"
            "#(ow,(12 + 15 = )##(++,(Fish 12),15)(\n))"
            "#(ow,(12 - 15 = )##(--,(Fish 12),15)(\n))"
            "#(ow,(12 * 15 = )##(**,(Fish 12),15)(\n))"
            "#(ow,(12 / 15 = )##(//,(Fish 12),15)(\n))"
            "#(ow,(12 % 15 = )##(%%,(Fish 12),15)(\n))"
            "#(ow,(12 & 15 = )##(&&,(Fish 12),15)(\n))"
            "#(ow,(12 | 15 = )##(||,(Fish 12),15)(\n))"
            "#(ow,(12 ^ 15 = )##(^^,(Fish 12),15)(\n))"
            "#(ow,#(g?,9,10,(#(ow,(9>10 true\n))),(#(ow,(9>10 false\n)))))"
            "#(ow,#(g?,10,9,(#(ow,(10>9 true\n))),(#(ow,(10>9 false\n)))))"
            "#(ow,(Before #(es,zz) ##(ls,(,),z) = )##(ls,(,),z)(\n))"
            "#(es,zz)"
            "#(ow,(After  #(es,zz) ##(ls,(,)) = )##(ls,(,))(\n))"
            "#(ow,(##(ct) = )##(ct)(\n))"
            "#(ow,(##(ct,mint) = )##(ct,mint)(\n))"
            "#(ow,(##(ct,.,z) = )##(ct,.,z)(\n))"
            "#(ow,(##(ct,mint,z) = )##(ct,mint,z)(\n))"
            "#(ow,(##(ct,/bin/cat,z) = )##(ct,/bin/cat,z)(\n))"
            "#(ow,(##(ct,/dev/null,z) = )##(ct,/dev/null,z)(\n))"
            "#(ds,test,(Test SELF,ARG1,ARG2,ARG3))#(mp,test,SELF,ARG1,ARG2,ARG3)"
            "#(ow,(Test mp: should be 'Test test,A,B,C' = ')##(test,A,B,C)('\n))"
            "#(ow,(Test hk: should be 'This is z1' = ')##(hk,aa,bb,cc,dd,z1)('\n))"
            "#(ds,xlat,(z0123456789))"
            "#(ow,(Test si: should be 'A0123456789Z' = ')"
            "##(si,xlat,(A\001\002\003\004\005\006\007\010\011\012Z))('\n))"
            "#(ow,(Test bc: should be '65' = ')##(bc,A)('\n))"
            "#(ow,(Test bc: should be '41' = ')##(bc,A,a,h)('\n))"
            "#(ow,(Test bc: should be '101' = ')##(bc,A,a,o)('\n))"
            "#(ow,(Test bc: should be '1000001' = ')##(bc,A,a,b)('\n))"
            "#(ow,(Test bc: should be 'Fish 41' = ')##(bc,Fish 65,d,h)('\n))"
            "#(ow,(Test bc: should be 'Fish 41' = ')##(bc,Fish 101,o,h)('\n))"
            "#(ow,(Test ff: ')##(ff,./mint*,(,))('\n))"
            "#(ow,(Test ff: ')##(ff,k*,(,))('\n))"
            "#(ow,(Test rn: ')##(rn,q,qq)('\n))"
            "#(ow,(Test rn: ')##(rn,qq,q)('\n))"
            "#(ow,(Test rn: ')##(rn,q,qq)('\n))"
            "#(ow,(Test de: ')##(de,qwerty)('\n))"
            "#(ow,(z: ')##(ls,(,),z)('\n))"
            "#(ow,(Test sl: ')##(sl,querty,#(ls,(,),z))('\n))"
            "#(ow,(Erase z*\n)##(es,#(ls,(,),z))"
            "#(ow,(z: ')##(ls,(,),z)('\n))"
            "#(ow,(Test ll: ')##(ll,querty)('\n))"
            "#(ow,(z: ')##(ls,(,),z)('\n))"
            "#(ev)"
            "#(ow,(Test ev: )##(ls,(,),env.)(\n))"
            "#(ow,(Test env.PWD: )##(env.PWD)(\n))"
            "#(ow,(Current buffer number: ')##(ba,-1)('\n))"
            "#(ds,buf,##(ba,-1))"
            "#(ow,(Create new buffer: ')##(ba,0)('\n))"
            "#(ow,(Current buffer number: ')##(ba,-1)('\n))"
            "#(ow,(Select old buffer: ')##(ba,##(buf))('\n))"
            "#(ow,(Current buffer number: ')##(ba,-1)('\n))"
            "#(ow,(Insert string 'hello' into buffer: )##(is,he,OK)( )#(is,llo,OK)(\n))"
            "#(pb)"
            "#(ow,(##(rm,[) should be 'hello': ')##(rm,[)('\n))"
            "#(ow,(##(rm,]) should be '': ')##(rm,])('\n))"
            "#(sp,<<<)"
            "#(ow,(##(rm,[) should be 'he': ')##(rm,[)('\n))"
            "#(ow,(##(rm,]) should be 'llo': ')##(rm,])('\n))"
            "#(ow,(##(rc,[) should be '2': ')##(rc,[)('\n))"
            "#(ow,(##(rc,]) should be '3': ')##(rc,])('\n))"
            "#(sp,[)#(wf,qwerty,])#(dm,])"
            "##(pb)##(rf,qwerty)##(pb)"
            "#(sp,[)#(dm,])#(pb)"
            "#(rf,qwerty)#(sp,]>>)"
            "##(pb)"
            "#(ow,(##(mb,<,True,False) should be 'True': ')##(mb,<,True,False)('\n))"
            "#(ow,(##(mb,>,True,False) should be 'False': ')##(mb,>,True,False)('\n))"
            "#(ow,(##(mb,.,True,False) should be 'False': ')##(mb,.,True,False)('\n))"
            "#(ow,(##(pm,1000,Oops) should be 'Oops': ')##(pm,1000,Oops)('\n))"
            "#(ow,(##(pm,10,Oops) should be '': ')##(pm,10,Oops)('\n))"
            "#(ow,(##(pm,10,Oops) should be '': ')##(pm,10,Oops)('\n))"
            "#(ow,(##(pm,10,Oops) should be '': ')##(pm,10,Oops)('\n))"
            "#(ow,(##(pm,10,Oops) should be '': ')##(pm,10,Oops)('\n))"
            "#(ow,(##(pm,10,Oops) should be 'Oops': ')##(pm,10,Oops)('\n))"
            "#(pm)#(pm)#(pm)"
            "#(ow,(##(pm,40,Oops) should be 'Oops': ')##(pm,40,Oops)('\n))"
            "#(ow,(##(pm,30,Oops) should be '': ')##(pm,30,Oops)('\n))"
            "#(pm)#(pm)#(pm)#(pm,10)"
            "#(sp,[)#(dm,])#(is,(This is a test string))"
            "#(sp,[>>>>)#(sm,0)#(sp,[)"
            "#(ow,(##(rm,0) should be 'This': ')##(rm,0)('\n))"
            "#(pm)"
            "#(ow,(##(rm,0) should be '': ')##(rm,0)('\n))"
            "#(sp,[>>>>)#(sm,@)#(sp,[)"
            "#(ow,(##(rm,@) should be 'This': ')##(rm,@)('\n))"
            );
        interp.addPrim("ow", new owPrim(std::cout));
        registerSysPrims(interp, 0, 0, 0);
        registerStrPrims(interp);
        registerFrmPrims(interp);
        registerMthPrims(interp);
        registerLibPrims(interp);

        registerBufPrims(interp);

        for (mintcount_t i = 0; i < 1; i++) {
            interp.scan();
        } // for
    } catch(std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    } // catch
    return 0;
} // main
    
