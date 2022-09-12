#include "partman.h"
#include "gdrv.h"
#include "memory.h"
#include "zdrv.h"

short partman::_field_size[] =
{
	2, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0
};

datFileStruct::~datFileStruct()
{
	for (int groupIndex = 0; groupIndex < NumberOfGroups; groupIndex++)
	{
		datGroupData* group = GroupData[groupIndex];
		if (group)
		{
			int entryIndex = 0;
			for (int entryIndex = 0; entryIndex < group->EntryCount; entryIndex++)
			{
				datEntryData* entry = &group->Entries[entryIndex];
				if (entry->Buffer)
				{
					if (entry->EntryType == datFieldTypes::Bitmap8bit)
						gdrv::destroy_bitmap((gdrv_bitmap8*)entry->Buffer);
					memory::free(entry->Buffer);
				}
			}
			memory::free(group);
		}
	}
	if (Description)
		memory::free(Description);
	memory::free(GroupData);
}

datFileStruct* partman::load_records(const char* lpFileName)
{
	_OFSTRUCT ReOpenBuff{};
	datFileHeader header{};
	dat8BitBmpHeader bmpHeader{};
	dat16BitBmpHeader zMapHeader{};

	const HFILE fileHandle = OpenFile(lpFileName, &ReOpenBuff, 0);
	if (fileHandle == -1)
		return nullptr;
	_lread(fileHandle, &header, 183u);
	if (lstrcmpA("PARTOUT(4.0)RESOURCE", header.FileSignature))
	{
		_lclose(fileHandle);
		return nullptr;
	}
	auto datFile = new datFileStruct{};
	if (!datFile)
	{
		_lclose(fileHandle);
		return nullptr;
	}
	if (lstrlenA(header.Description) <= 0)
	{
		datFile->Description = nullptr;
	}
	else
	{
		int lenOfStr = lstrlenA(header.Description);
		auto descriptionBuf = static_cast<char*>(memory::allocate(lenOfStr + 1));
		datFile->Description = descriptionBuf;
		if (!descriptionBuf)
		{
			_lclose(fileHandle);
			delete datFile;
			return nullptr;
		}
		lstrcpyA(descriptionBuf, header.Description);
	}

	if (header.Unknown)
	{
		auto unknownBuf = static_cast<char*>(memory::allocate(header.Unknown));
		if (!unknownBuf)
		{
			_lclose(fileHandle);
			delete datFile;
			return nullptr;
		}
		_lread(fileHandle, static_cast<void*>(unknownBuf), header.Unknown);
		memory::free(unknownBuf);
	}

	auto groupDataBuf = (datGroupData**)memory::allocate(sizeof(void*) * header.NumberOfGroups);
	datFile->GroupData = groupDataBuf;
	if (!groupDataBuf)
	{
		delete datFile;
		return nullptr;
	}

	bool abort = false;
	for (auto groupIndex = 0; !abort && groupIndex < header.NumberOfGroups; ++groupIndex)
	{
		auto entryCount = _lread_char(fileHandle);
		auto groupDataSize = entryCount <= 0 ? 0 : entryCount - 1;
		auto groupData = reinterpret_cast<datGroupData*>(memory::allocate(
			sizeof(datEntryData) * groupDataSize + sizeof(datGroupData)));
		datFile->GroupData[groupIndex] = groupData;
		if (!groupData)
			break;

		groupData->EntryCount = entryCount;
		for (int entryIndex = 0; entryIndex < entryCount; entryIndex++)
		{
			datEntryData* entryData = &groupData->Entries[entryIndex];
			auto entryType = static_cast<datFieldTypes>(_lread_char(fileHandle));
			entryData->EntryType = entryType;

			int fieldSize = _field_size[static_cast<int>(entryType)];
			if (fieldSize < 0)
				fieldSize = _lread_long(fileHandle);

			if (entryType == datFieldTypes::Bitmap8bit)
			{
				_hread(fileHandle, &bmpHeader, sizeof(dat8BitBmpHeader));
				auto bmp = reinterpret_cast<gdrv_bitmap8*>(memory::allocate(sizeof(gdrv_bitmap8)));
				entryData->Buffer = reinterpret_cast<char*>(bmp);
				if (!bmp)
				{
					abort = true;
					break;
				}
				if (bmpHeader.IsFlagSet(bmp8Flags::DibBitmap)
					    ? gdrv::create_bitmap(bmp, bmpHeader.Width, bmpHeader.Height)
					    : gdrv::create_raw_bitmap(bmp, bmpHeader.Width, bmpHeader.Height,
					                              bmpHeader.IsFlagSet(bmp8Flags::RawBmpUnaligned)))
				{
					abort = true;
					break;
				}
				_hread(fileHandle, bmp->BmpBufPtr1, bmpHeader.Size);
				bmp->XPosition = bmpHeader.XPosition;
				bmp->YPosition = bmpHeader.YPosition;
			}
			else if (entryType == datFieldTypes::Bitmap16bit)
			{
				_hread(fileHandle, &zMapHeader, sizeof(dat16BitBmpHeader));
				int length = fieldSize - sizeof(dat16BitBmpHeader);
				
				auto zmap = reinterpret_cast<zmap_header_type*>(memory::allocate(sizeof(zmap_header_type) + length));
				zmap->Width = zMapHeader.Width;
				zmap->Height = zMapHeader.Height;
				zmap->Stride = zMapHeader.Stride;
				_hread(fileHandle, zmap->ZBuffer, length);
				entryData->Buffer = reinterpret_cast<char*>(zmap);
			}
			else
			{
				char* entryBuffer = static_cast<char*>(memory::allocate(fieldSize));
				entryData->Buffer = entryBuffer;
				if (!entryBuffer)
				{
					abort = true;
					break;
				}
				_hread(fileHandle, entryBuffer, fieldSize);
			}

			entryData->FieldSize = fieldSize;
			datFile->NumberOfGroups = groupIndex + 1;
		}
	}

	_lclose(fileHandle);
	if (datFile->NumberOfGroups == header.NumberOfGroups)
		return datFile;
	unload_records(datFile);
	return nullptr;
}


void partman::unload_records(datFileStruct* datFile)
{
	delete datFile;
}

char* partman::field(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType)
{
	datGroupData* groupData = datFile->GroupData[groupIndex];
	for (int entryIndex = 0; entryIndex < groupData->EntryCount; entryIndex++)
	{
		auto entry = &groupData->Entries[entryIndex];
		if (entry->EntryType == targetEntryType)
		{
			return entry->Buffer;
		}
	}
	return nullptr;
}


char* partman::field_nth(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType, int skipFirstN)
{
	datGroupData* groupData = datFile->GroupData[groupIndex];
	int skipCount = 0;
	for (int entryIndex = 0; entryIndex < groupData->EntryCount; entryIndex++)
	{
		datEntryData* entry = &groupData->Entries[entryIndex];
		auto entryType = entry->EntryType;
		if (entryType == targetEntryType)
		{
			if (skipCount == skipFirstN)
			{
				return entry->Buffer;
			}
			else
			{
				skipCount++;
			}
		}
		else
		{
			if (targetEntryType < entryType)
			{
				return nullptr;
			}
		}
	}

	return nullptr;
}

int partman::field_size_nth(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType, int skipFirstN)
{
	datGroupData* groupData = datFile->GroupData[groupIndex];
	int skipCount = 0;
	for (int entryIndex = 0; entryIndex < groupData->EntryCount; entryIndex++)
	{
		datEntryData* entry = &groupData->Entries[entryIndex];
		auto entryType = entry->EntryType;
		if (entryType == targetEntryType)
		{
			if (skipCount == skipFirstN)
			{
				return entry->FieldSize;
			}
			else
			{
				skipCount++;
			}
		}
		else
		{
			if (targetEntryType < entryType)
			{
				return 0;
			}
		}
	}

	return 0;
}

int partman::field_size(datFileStruct* datFile, int groupIndex, datFieldTypes targetEntryType)
{
	return field_size_nth(datFile, groupIndex, targetEntryType, 0);
}

int partman::record_labeled(datFileStruct* datFile, const char* targetGroupName)
{
	int trgGroupNameLen = lstrlenA(targetGroupName);
	int groupIndex = datFile->NumberOfGroups;
	while (true)
	{
		if (--groupIndex < 0)
			return -1;
		char* groupName = field(datFile, groupIndex, datFieldTypes::GroupName);
		if (groupName)
		{
			int index = 0;
			bool found = trgGroupNameLen == 0;
			if (trgGroupNameLen > 0)
			{
				const char* targetNamePtr = targetGroupName;
				do
				{
					if (*targetNamePtr != targetNamePtr[groupName - targetGroupName])
						break;
					++index;
					++targetNamePtr;
				}
				while (index < trgGroupNameLen);
				found = index == trgGroupNameLen;
			}
			if (found && !targetGroupName[index] && !groupName[index])
				break;
		}
	}
	return groupIndex;
}

char* partman::field_labeled(datFileStruct* datFile, const char* lpString, datFieldTypes fieldType)
{
	char* result;
	int groupIndex = record_labeled(datFile, lpString);
	if (groupIndex < 0)
		result = nullptr;
	else
		result = field(datFile, groupIndex, fieldType);
	return result;
}

char partman::_lread_char(HFILE hFile)
{
	char Buffer = 0;
	_lread(hFile, &Buffer, 1u);
	return Buffer;
}

int partman::_lread_long(HFILE hFile)
{
	int Buffer = 0;
	_lread(hFile, &Buffer, 4u);
	return Buffer;
}
