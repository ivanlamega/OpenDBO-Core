#include "stdafx.h"
#include "FriendRepository.h"
#include "../QueryServer.h"


void CFriendRepository::DeleteFriendList(CHARACTERID charId)
{
	GetCharDB.Execute("DELETE FROM friendlist WHERE char_id=%u OR friend_char_id=%u", charId, charId);
}

void CFriendRepository::UpdateFriendName(WCHAR* wszName, CHARACTERID friendId)
{
	GetCharDB.Execute("UPDATE friendlist SET friend_name=\"%ls\" WHERE friend_char_id=%u", wszName, friendId);
}
