/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2017 Hellfire <https://hellfire-core.github.io/>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Boss_Nefarian
SD%Complete: 80
SDComment: Some issues with class calls effecting more than one class, his throne is actually a portal -_-, should be changed
SDCategory: Blackwing Lair
EndScriptData */

#include "scriptPCH.h"
#include "def_blackwing_lair.h"

struct boss_nefarianAI : public ScriptedAI
{
    boss_nefarianAI(Creature* c) : ScriptedAI(c)
    {
        instance = c->GetInstanceData();
    }

    ScriptedInstance* instance;
    EventMap events;

    enum Events
    {
        EVENT_CAST_SHADOW_FLAME = 1,
        EVENT_CAST_BELLOWING_ROAR,
        EVENT_CAST_VEIL_OF_SHADOW,
        EVENT_CAST_CLEAVE,
        EVENT_CAST_TAIL_LASH,
        EVENT_CAST_CLASS_CALL,
        EVENT_RISE_SKELETONS
    };

    enum Spells
    {
        SPELL_SHADOWFLAME_INITIAL   = 22992,
        SPELL_SHADOWFLAME           = 22539,
        SPELL_BELLOWINGROAR         = 22686,
        SPELL_VEILOFSHADOW          = 22687,
        SPELL_CLEAVE                = 20691,
        SPELL_TAILLASH              = 23364,
        SPELL_BONECONTRUST          = 23363,                   //23362, 23361

        SPELL_MAGE                  = 23410,                   //wild magic
        SPELL_WARRIOR               = 23397,                   //beserk
        SPELL_DRUID                 = 23398,                   // cat form
        SPELL_PRIEST                = 23401,                   // corrupted healing
        SPELL_PALADIN               = 23418,                   //syphon blessing
        SPELL_SHAMAN                = 23425,                   //totems
        SPELL_WARLOCK               = 23427,                   //infernals
        SPELL_HUNTER                = 23436,                   //bow broke
        SPELL_ROGUE                 = 23414,                   //Paralise
    };

    enum Texts
    {
        SAY_AGGRO                   = -1469007,
        SAY_XHEALTH                 = -1469008,
        SAY_SHADOWFLAME             = -1469009,
        SAY_RAISE_SKELETONS         = -1469010,
        SAY_SLAY                    = -1469011,
        SAY_DEATH                   = -1469012,

        SAY_MAGE                    = -1469013,
        SAY_WARRIOR                 = -1469014,
        SAY_DRUID                   = -1469015,
        SAY_PRIEST                  = -1469016,
        SAY_PALADIN                 = -1469017,
        SAY_SHAMAN                  = -1469018,
        SAY_WARLOCK                 = -1469019,
        SAY_HUNTER                  = -1469020,
        SAY_ROGUE                   = -1469021,
    };

    void IsSummonedBy(Unit *summoner)
    {
        me->SetLevitate(true);
        me->SetWalk(false);

        me->SetHomePosition(-7493.8f, -1260.0f, 476.8f, 0.0f);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MovePoint(0, -7493.8f, -1260.0f, 476.8f);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == 0)
        {
            DoZoneInCombat();
            AttackStart(SelectUnit(SELECT_TARGET_RANDOM, 0));
        }
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat();

        me->SetLevitate(false);
        me->SetWalk(true);
        me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);

        DoCast(who,SPELL_SHADOWFLAME_INITIAL);
        DoScriptText(RAND(SAY_XHEALTH, SAY_AGGRO, SAY_SHADOWFLAME), me);

        events.Reset()
              .ScheduleEvent(EVENT_CAST_SHADOW_FLAME, 12000)
              .ScheduleEvent(EVENT_CAST_BELLOWING_ROAR, 30000)
              .ScheduleEvent(EVENT_CAST_VEIL_OF_SHADOW, 15000)
              .ScheduleEvent(EVENT_CAST_CLEAVE, 7000)
              .ScheduleEvent(EVENT_CAST_TAIL_LASH, 10000)
              .ScheduleEvent(EVENT_CAST_CLASS_CALL, 35000);
    }

    void KilledUnit(Unit* Victim)
    {
        if (rand()%5 == 0)
            DoScriptText(SAY_SLAY, m_creature, Victim);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (instance)
            instance->SetData(DATA_NEFARIAN_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
            return;

        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_EVERYTHING);

        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CAST_SHADOW_FLAME:
                {
                    DoCast(m_creature->GetVictim(),SPELL_SHADOWFLAME);
                    events.ScheduleEvent(EVENT_CAST_SHADOW_FLAME, 12000);
                    break;
                }
                case EVENT_CAST_BELLOWING_ROAR:
                {
                    DoCast(m_creature->GetVictim(), SPELL_BELLOWINGROAR);
                    events.ScheduleEvent(EVENT_CAST_BELLOWING_ROAR, 30000);
                    break;
                }
                case EVENT_CAST_VEIL_OF_SHADOW:
                {
                    DoCast(m_creature->GetVictim(),SPELL_VEILOFSHADOW);
                    events.ScheduleEvent(EVENT_CAST_VEIL_OF_SHADOW, 15000);
                    break;
                }
                case EVENT_CAST_CLEAVE:
                {
                    DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
                    events.ScheduleEvent(EVENT_CAST_CLEAVE, 7000);
                    break;
                }
                case EVENT_CAST_TAIL_LASH:
                {
                    DoCast(m_creature->GetVictim(),SPELL_TAILLASH); // TODO: test whether Tail Lash works correctly
                    events.ScheduleEvent(EVENT_CAST_TAIL_LASH, 10000);
                    break;
                }
                case EVENT_CAST_CLASS_CALL:
                {
                    if (Unit* player = SelectUnit(SELECT_TARGET_RANDOM, 0, 300.0f, true))
                    {
                        // switch (urand(1, 11)) // debug only
                        switch (player->GetClass())
                        {
                            //
                            // working
                            //

                            case CLASS_DRUID: // force Cat Form
                                DoScriptText(SAY_DRUID, m_creature);
                                DoCast(m_creature,SPELL_DRUID);
                                break;
                            case CLASS_HUNTER: // destroy ranged weapon
                                DoScriptText(SAY_HUNTER, m_creature);
                                DoCast(m_creature,SPELL_HUNTER);
                                break;
                            case CLASS_WARRIOR: // force Berserker Stance
                                DoScriptText(SAY_WARRIOR, m_creature);
                                DoCast(m_creature,SPELL_WARRIOR);
                                break;
                            case CLASS_ROGUE: // paralyze
                                DoScriptText(SAY_ROGUE, m_creature);
                                DoCast(m_creature,SPELL_ROGUE);
                                // TODO: teleport player in front of boss
                                break;

                            //
                            // not working
                            //

                            case CLASS_MAGE: // Wild Polymorph
                                DoScriptText(SAY_MAGE, m_creature);
                                DoCast(m_creature,SPELL_MAGE);
                                break;
                            case CLASS_PRIEST: // DoT on direct heals
                                DoScriptText(SAY_PRIEST, m_creature);
                                DoCast(m_creature,SPELL_PRIEST);
                                break;
                            case CLASS_PALADIN: // melee bubble
                                DoScriptText(SAY_PALADIN, m_creature);
                                DoCast(m_creature,SPELL_PALADIN);
                                break;
                            case CLASS_SHAMAN: // totems for Nefarian
                                DoScriptText(SAY_SHAMAN, m_creature);
                                DoCast(m_creature,SPELL_SHAMAN);
                                break;
                            case CLASS_WARLOCK: // infernals (14668)
                                DoScriptText(SAY_WARLOCK, m_creature);
                                DoCast(m_creature,SPELL_WARLOCK);
                                break;
                        }
                    }
        
                    events.ScheduleEvent(EVENT_CAST_CLASS_CALL, urand(35000, 40000));
                    break;
                }
                case EVENT_RISE_SKELETONS:
                {
                    DoScriptText(SAY_RAISE_SKELETONS, m_creature);
                    // TODO: implement phase 3 at 20% health
                    break;
                }
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_nefarian(Creature *_Creature)
{
    return new boss_nefarianAI (_Creature);
}

void AddSC_boss_nefarian()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_nefarian";
    newscript->GetAI = &GetAI_boss_nefarian;
    newscript->RegisterSelf();
}
